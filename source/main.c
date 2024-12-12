#include <switch.h>        
#include <stdlib.h>      
#include <string.h>       
#include <stdio.h>         

#define MAX_PADS 8  // Maximum number of controllers

// Global variables
HidsysUniquePadId successfulPads[MAX_PADS];  // Array to store successful UniquePadIds
int numSuccessfulPads = 0;  // Count of successful controllers
PadState pad[MAX_PADS];  // State for each controller
HidsysNotificationLedPattern   pattern;  // LED pattern
int selectedController = 1;  // Index of the currently selected controller in the successfulPads array
bool rButtonPressed = false;  // To detect when 'R' is pressed

// Function to display the menu
void printMenu() {
printf(" \n ");       // Atrocious formatting (The Switch was being weird)
printf(" \n ");
printf("\n Connected controllers: %d\n \n", numSuccessfulPads);
    printf(" \n ");
    printf(" \n ");
printf(" Use the Joy-Con attached to the console, or the Player 1 controller\n\n  to modify the LED of the right Joy-Con or Pro Controller. \n ");
printf(" \n ");
printf(" \n ");
printf(" \n ");
printf(" \n ");
    printf("    Press 'A' for solid LED on.\n \n");
printf(" \n ");
    printf("    Press 'B' for dim brightness.\n \n");
printf(" \n ");
    printf("    Press 'X' for Fading.\n \n");
printf(" \n ");
    printf("    Press 'Y' for OFF.\n \n");
printf(" \n ");
    printf("    Press '+' to return to Homebrew Meun.\n \n");
printf(" \n "); 
    printf("    Press 'R' to cycle controllers.\n");
printf(" \n ");
printf(" \n ");
printf(" \n ");
printf(" \n ");  
printf(" \n ");
printf(" \n "); 
}
         
void removeController(int index) {
    if (index < 0 || index >= numSuccessfulPads) return; // Invalid index
    // Shift remaining controllers in the array
    for (int i = index; i < numSuccessfulPads - 1; i++) {
        successfulPads[i] = successfulPads[i + 1];
    }
    numSuccessfulPads--;

    // Adjust selectedController to stay within bounds
    if (selectedController >= numSuccessfulPads) {
        selectedController = (numSuccessfulPads > 0) ? numSuccessfulPads - 1 : 0;
    }
}


void updateConnectedControllers() {
    HidNpadIdType controllerTypes[MAX_PADS] = {HidNpadIdType_Handheld,
        HidNpadIdType_No1, HidNpadIdType_No2, HidNpadIdType_No3, HidNpadIdType_No4,
        HidNpadIdType_No5, HidNpadIdType_No6, HidNpadIdType_No7};

    for (int i = 0; i < MAX_PADS; i++) {
        HidsysUniquePadId padIds[MAX_PADS];
        s32 total_entries = 0;

        // Retrieve UniquePadIds for each controller type
        Result rc = hidsysGetUniquePadsFromNpad(controllerTypes[i], padIds, MAX_PADS, &total_entries);

        if (R_SUCCEEDED(rc) && total_entries > 0) {
            for (int j = 0; j < total_entries; j++) {
                // Check if this controller is already in successfulPads
                bool alreadyAdded = false;
                for (int k = 0; k < numSuccessfulPads; k++) {
                    if (memcmp(&successfulPads[k], &padIds[j], sizeof(HidsysUniquePadId)) == 0) {
                        alreadyAdded = true;
                        break;
                    }
                }

                // If not already added, try setting a dim LED mode to confirm functionality
                if (!alreadyAdded) {
                    rc = hidsysSetNotificationLedPattern(&pattern, padIds[j]);
                    if (R_SUCCEEDED(rc)) {
                        successfulPads[numSuccessfulPads++] = padIds[j];
                        printf("    Controller added. Total: %d    ", numSuccessfulPads);
                    }
                }
            }
        }
    }
}

// LED control functions
void setLedSolidOn(HidsysUniquePadId padId) {
    memset(&pattern, 0, sizeof(pattern));
    pattern.baseMiniCycleDuration = 0x0F;
    pattern.startIntensity = 0xF;
    pattern.miniCycles[0].ledIntensity = 0xF;
    pattern.miniCycles[0].transitionSteps = 0x0F;
    pattern.miniCycles[0].finalStepDuration = 0x0F;

    Result rc = hidsysSetNotificationLedPattern(&pattern, padId);
    consoleClear();
    printMenu();
    if (R_SUCCEEDED(rc)) {
        printf("    LED set to ON\n");
    } else {
         removeController(selectedController);
    }
}

void setLedDim(HidsysUniquePadId padId) {
    memset(&pattern, 0, sizeof(pattern));
    pattern.baseMiniCycleDuration = 0x0F;
    pattern.startIntensity = 0x5;
    pattern.miniCycles[0].ledIntensity = 0x5;
    pattern.miniCycles[0].transitionSteps = 0x0F;
    pattern.miniCycles[0].finalStepDuration = 0x0F;

    Result rc = hidsysSetNotificationLedPattern(&pattern, padId);
    consoleClear();
    printMenu();
    if (R_SUCCEEDED(rc)) {
        printf("    LED set to dim brightness\n");
    } else {
         removeController(selectedController);
    }
}

void setLedFading(HidsysUniquePadId padId) {
    memset(&pattern, 0, sizeof(pattern));
    pattern.baseMiniCycleDuration = 0x8;
    pattern.totalMiniCycles = 0x2;
    pattern.startIntensity = 0x2;
    pattern.miniCycles[0].ledIntensity = 0xF;
    pattern.miniCycles[0].transitionSteps = 0xF;
    pattern.miniCycles[1].ledIntensity = 0x2;
    pattern.miniCycles[1].transitionSteps = 0xF;

    Result rc = hidsysSetNotificationLedPattern(&pattern, padId);
    consoleClear();
    printMenu();
    if (R_SUCCEEDED(rc)) {
        printf("    LED set to fading\n");
    } else {
          removeController(selectedController);
    }
}

void setLedOff(HidsysUniquePadId padId) {
    memset(&pattern, 0, sizeof(pattern));
    Result rc = hidsysSetNotificationLedPattern(&pattern, padId);
    consoleClear();
    printMenu();
    if (R_SUCCEEDED(rc)) {
        printf("    LED set to OFF\n");
    } else {
        removeController(selectedController);
    }
}

// Function to check all controllers for LED support
void initialCheckControllers() {
    HidNpadIdType controllerTypes[MAX_PADS] = {HidNpadIdType_Handheld,
        HidNpadIdType_No1, HidNpadIdType_No2, HidNpadIdType_No3, HidNpadIdType_No4,
        HidNpadIdType_No5, HidNpadIdType_No6, HidNpadIdType_No7};

    for (int i = 0; i < MAX_PADS; i++) {
        HidsysUniquePadId padIds[MAX_PADS];
        s32 total_entries = 0;

        // Retrieve UniquePadIds for each controller type
        Result rc = hidsysGetUniquePadsFromNpad(controllerTypes[i], padIds, MAX_PADS, &total_entries);

        if (R_SUCCEEDED(rc) && total_entries > 0) {
            for (int j = 0; j < total_entries; j++) {
                // Check if this padId is already registered
                bool alreadyRegistered = false;
                for (int k = 0; k < numSuccessfulPads; k++) {
                    if (memcmp(&successfulPads[k], &padIds[j], sizeof(HidsysUniquePadId)) == 0) {
                        alreadyRegistered = true;
                        break;
                    }
                }
                // Add new controller if not already registered
                if (!alreadyRegistered) {
                    rc = hidsysSetNotificationLedPattern(&pattern, padIds[j]);
                    if (R_SUCCEEDED(rc)) {
                        successfulPads[numSuccessfulPads++] = padIds[j];
                    }
                }
            }
        }
    }
}

// Controller cycling logic
void cycleController() {
    if (numSuccessfulPads > 0) {
        // Increment selected controller and wrap around
        selectedController = (selectedController + 1) % numSuccessfulPads;

        consoleClear();
        printMenu();
       
       for (int i = 0; i < numSuccessfulPads; i++) {
    if (selectedController == i) {
        printf("    Switched to controller %d\n", i + 1);
    }
}

    } else {
        printf("No controllers available to cycle.\n");
    }
}

// Main program
int main(int argc, char* argv[]) {
    consoleInit(NULL);
    padConfigureInput(MAX_PADS, HidNpadStyleSet_NpadStandard);

    // Initialize pad states
    for (int i = 0; i < MAX_PADS; i++) {
        padInitializeDefault(&pad[i]);
    }

    Result rc = hidsysInitialize();
    if (R_FAILED(rc)) {
        printf("Failed to initialize hidsys (Error: 0x%x)\n", rc);
        consoleExit(NULL);
        return -1;
    }

    // Initial check of connected controllers
    initialCheckControllers();

    printMenu();
    // Start with the first detected controller
    if (numSuccessfulPads > 0) {
        selectedController = 0;  // Ensure we start with the attached Joy-Con (if present)
    } else {
        printf("No controllers detected with LED capability.\n");
    }
initialCheckControllers();
   while (appletMainLoop()) {
    // Update the list of connected controllers
    updateConnectedControllers();

    if (numSuccessfulPads == 0) {
        printf("No controllers available with LED capability.\n");
        break;
    }

    padUpdate(&pad[selectedController]);
    u64 kDownAll = padGetButtonsDown(&pad[selectedController]);

    // Handle LED control inputs
    if (kDownAll & HidNpadButton_A) setLedSolidOn(successfulPads[selectedController]);
    if (kDownAll & HidNpadButton_B) setLedDim(successfulPads[selectedController]);
    if (kDownAll & HidNpadButton_X) setLedFading(successfulPads[selectedController]);
    if (kDownAll & HidNpadButton_Y) setLedOff(successfulPads[selectedController]);

    // Cycle controllers on 'R'
    if (kDownAll & HidNpadButton_R) {
        if (!rButtonPressed) {
            cycleController();
            rButtonPressed = true;
        }
    } else {
        rButtonPressed = false;
    }

    // Exit on '+'
    if (kDownAll & HidNpadButton_Plus) break;

    consoleUpdate(NULL);
}
    hidsysExit();
    consoleExit(NULL);
    return 0;
}