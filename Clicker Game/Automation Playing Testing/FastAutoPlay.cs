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

public class FastAutoPlay : MonoBehaviour
{
    public GameObject GameManagerObject;
    public GameObject TelemetryObject;

    public Button FishGatherButton;
    public Button FeedSoldierButton;

    public Toggle fastPlayToggle;

    public GameObject Phase2;
    public GameObject Phase3;
    public GameObject Phase4;
    public GameObject Phase5;
    public int PhaseCoutner = 1;

    public Button[] allButtons;

    [Header("Phase Probability Section")]
    [Header("________________________________________")]

    public List<float> WoodPhaseProbability = new List<float>();
    public List<float> FishPhaseProbability = new List<float>();
    public List<float> CobaltPhaseProbability = new List<float>();
    public List<float> SoldierPhaseProbability = new List<float>();

    [Header("In Play Probability Section")]
    [Header("________________________________________")]

    public List<float> allProbability = new List<float>();

    public List<Button> canPlayButtons = new List<Button>();
    public List<float> canPlayProbability = new List<float>();

    float totalProbability = 0.0f;
    public int turnsToFeed;
    public int soldierAmount;


    // Start is called before the first frame update
    void Start()
    {
        canPlayButtons.Clear();
        canPlayProbability.Clear();
        totalProbability = 0.0f;
    }

    // Update is called once per frame
    void Update()
    {
        //if game is won turn off autoplay
        GameManager GameManager = GameManagerObject.GetComponent<GameManager>();
        Telemetry Telemetry = TelemetryObject.GetComponent<Telemetry>();

        bool gameWon = GameManager.IsWon;

        if (gameWon == true)
        {
            fastPlayToggle.isOn = false;
        }
        else
        {
            //clear the lists incase an option is no longer clickable
            canPlayButtons.Clear();
            canPlayProbability.Clear();
            totalProbability = 0.0f;

            //if toggled on
            if (fastPlayToggle.isOn)
            {
                //check phase and change prob if needed
                PhaseChangeProbability();

                //if in soldier phase do this
                if (PhaseCoutner >= 4 )
                {

                    turnsToFeed = GameManager.turnsLeftToFeed;
                    soldierAmount = GameManager.SoldierAmount;

                    if (turnsToFeed <= 6 && soldierAmount >= 1)
                    {

                        if (GameManager.FishAmount < GameManager.feedSoldiersFishCost)
                        {
                            //click fishgather
                            FishGatherButton.onClick.Invoke();
                        }
                        else
                        {
                            //click button to feed soldiers
                            FeedSoldierButton.onClick.Invoke();
                        }
                    }
                    else
                    {
                        ButtonSelect();
                    }
                }
                else
                {
                    //do whatever
                    ButtonSelect();
                }
            }
        }
    }


    void PhaseChangeProbability()
    {
        GameManager GameManager = GameManagerObject.GetComponent<GameManager>();
        Telemetry Telemetry = TelemetryObject.GetComponent<Telemetry>();

        int phaseCounter = Telemetry.PhaseCoutner;

        switch (phaseCounter)
        {
            case 1:
                //Wood phase
                allProbability = WoodPhaseProbability;
                PhaseCoutner = 1;
                break;
            case 2:
                //Fish phase
                allProbability = FishPhaseProbability;
                PhaseCoutner = 2;
                break;
            case 3:
                //Cobalt phase
                allProbability = CobaltPhaseProbability;
                PhaseCoutner = 3;
                break;
            case 4:
                //Soldier phase
                allProbability = SoldierPhaseProbability;
                if(GameManager.GeneralAmount >= 12)
                {
                    SoldierPhaseProbability[17] = 0;
                }
                PhaseCoutner = 4;
                break;
            case 5:
                PhaseCoutner = 5;
                break;
        }

    }

    void ButtonSelect()
    {
        int currentButtonNumber = 0;
        int canPlayProbabilityNumber = 0;
        //check all currently active button
        foreach (Button button in allButtons)
        {

            //check for all active and interactable button and put it in a list
            if (button.gameObject.activeInHierarchy && button.interactable)
            {
                //if button prob is not zero then add to can play
                if (allProbability[currentButtonNumber] != 0.0f)
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

        switch (PhaseCoutner)
        {
            case 1:
                if (Phase2.activeInHierarchy)
                {
                    PhaseCoutner = 2;
                    fastPlayToggle.isOn = false;
                }
                break;
            case 2:
                if (Phase3.activeInHierarchy)
                {
                    PhaseCoutner = 3;
                    fastPlayToggle.isOn = false;
                }
                break;
            case 3:
                if (Phase4.activeInHierarchy)
                {
                    PhaseCoutner = 4;
                    fastPlayToggle.isOn = false;
                }
                break;


        }
    }
}
