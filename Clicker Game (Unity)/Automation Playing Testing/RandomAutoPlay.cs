using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Security.Cryptography.X509Certificates;
using System.Threading;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class RandomAutoPlay : MonoBehaviour
{
    public GameObject GameManagerObject;

    public Toggle randomPlayToggle;
    public Toggle SpeedModeToggle;
    public Button[] allButtons;
    public List<float> allProbability = new List<float>();

    public List<Button> canPlayButtons = new List<Button>();
    public List<float> canPlayProbability = new List<float>();

    float totalProbability = 0.0f;
    public bool isSlowFlag = true;
    bool isWaiting = false;

    IEnumerator WaitCoroutine()
    {
        // Wait for 0.5 seconds
        float countdownTime = 0.25f;

        while (countdownTime > 0)
        {
            countdownTime -= Time.deltaTime;
            yield return null; // Wait for the next frame
        }

        // happens after countdown is done
        PlayRandomButton();
        isWaiting = false;
    }

    // Start is called before the first frame update
    void Start()
    {
        StartCoroutine(WaitCoroutine());
        canPlayButtons.Clear();
        canPlayProbability.Clear();
        totalProbability = 0.0f;
    }

    // Update is called once per frame
    void Update()
    {
        isSlowFlag = !(SpeedModeToggle.isOn);

        //if game is won turn off autoplay
        GameManager GameManager = GameManagerObject.GetComponent<GameManager>();

        bool gameWon = GameManager.IsWon;

        if (gameWon == true)
        {
            randomPlayToggle.isOn = false;
        }
        else
        {
            //clear the lists incase an option is no longer clickable
            canPlayButtons.Clear();
            canPlayProbability.Clear();
            totalProbability = 0.0f;

            //if toggled on
            if (randomPlayToggle.isOn && !isWaiting)
            {
                if (isSlowFlag == true)
                {
                    isWaiting = true;
                    StartCoroutine(WaitCoroutine());
                }
                else
                {
                    PlayRandomButton();
                }
            }
            
        }
    }

    void PlayRandomButton()
    {
        int currentButtonNumber = 0;
        int canPlayProbabilityNumber = 0;
        //check all currently active button
        foreach (Button button in allButtons)
        {

            //check for all active and interactable button and put it in a list
            if (button.gameObject.activeInHierarchy && button.interactable)
            {
                //add the button to canPlayButtons list
                canPlayButtons.Add(button);
                //add the probability value to the can play probability
                canPlayProbability.Add(allProbability[currentButtonNumber]);

                //UnityEngine.Debug.Log(currentButtonNumber);

                //keep track of the total probability in the canPlayProbability list
                totalProbability += canPlayProbability[canPlayProbabilityNumber];
                //increament the amount of canPlayProbability in the list
                ++canPlayProbabilityNumber;

            }
            //increment the button location its looking at
            ++currentButtonNumber;
        }

        float randomPickProbability = 0.0f;

        if (canPlayProbability.Count > 0)
        {
            //choose the button in the list to click based on the probaboloty list
            randomPickProbability = UnityEngine.Random.Range(0, totalProbability);
        }

        //current button probability 
        float currentProbability = 0.0f;
        for (int i = 0; i < canPlayButtons.Count; ++i)
        {
            currentProbability += canPlayProbability[i];
            if (randomPickProbability <= currentProbability)
            {
                //simulate a button click
                canPlayButtons[i].onClick.Invoke();
            }
        }
    }


}
