#include <LPC17xx.h>
#include <stdio.h>
#include <string.h>

// Function prototypes
void display();
void port_write();
void lcd_write();
void clock_wise(void);
void anti_clock_wise(void);
void keyboard();

// Global variables
int temp1, temp2, flag1, i, j, k, l, m; // General purpose variables
int row, x, col; // Variables for keyboard row and column
unsigned long int var1, var2; // variables for stepping.
int pass_len = 0, sense = -1; // Password length and sensor status
char msg1[16]; // LCD message buffer
char msg2[16]; // LCD message buffer
int count = 0; // Headcount variable
int entrf = 0, exitf = 0; // Entry and exit flags
int exted = 0, entred = 0; // Entry and exit flags
int command[] = {0x30, 0x30, 0x30, 0x20, 0x28, 0x01, 0x06, 0x00, 0x80}; // LCD commands
char pass[32]; // Password buffer
long int seven_n[4][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 0xa, 0xb}, {0xc, 0xd, 0xe, 0xf}}; // Keyboard matrix
char message[] = "DIGITAL VAULT"; // Welcome message
char yes[] = "Correct Password"; // Message for correct password
char no[] = "Wrong Password"; // Message for wrong password
char clear[] = ""; // clear message
char enter_pass[] = "Enter Password"; // Prompt for password entry

// Main function
int main() {
    SystemInit(); // Initialize system
    SystemCoreClockUpdate(); // Update system core clock
    LPC_PINCON->PINSEL1 = 0;
    // Set pin select register 1 to 0
    LPC_PINCON->PINSEL0 = 0;
    // Set pin select register 0 to 0
    LPC_PINCON->PINSEL3 = 8;
    // Set pin select register 3 to 0
    LPC_PINCON->PINSEL4 = 0; // Set pin select register 4 to 0
    LPC_GPIO0->FIODIR = 0XFFFF7FFF; // Set GPIO port 0 direction for LCD
    LPC_GPIO2->FIODIR = 0XF << 10; // Set GPIO port 2 direction for keyboard row
    flag1 = 0; // Initialize flag1
    for (i = 0; i < 9; i++) { // Loop through LCD commands
        temp1 = command[i];
        lcd_write();
    }
    while (1) { // Main loop
        redirect: // Label for redirection
        flag1 = 0; // Initialize flag1
        temp1 = 0x01; // Set LCD command
        lcd_write();
        i = 0;
        while (message[i] != '\0') { // Loop through welcome message
            if (i == 16) {
                flag1 = 0;
                temp1 = 0xC0; // Move LCD cursor to second line
                lcd_write();
            }
            flag1 = 1;
            temp1 = message[i];
            lcd_write();
            for (k = 0; k < 500000; k++); // Delay
            i++;
        }
        for (k = 0; k < 5000; k++);
        LPC_GPIO0->FIOSET = 0XF << 4; // Activate LCD backlight
        for (k = 0; k < 5000; k++);
        keyboard(); // call keyboard function to go to password input phase
        goto redirect; // Redirect to welcome message
    }
    LPC_GPIO0->FIOCLR = 0XFF << 4; // Clear LCD backlight
    for (k = 0; k < 10; k++);
}

// Function to display messages on LCD
void display() {
    flag1 = 0;
    for (int i = 0; i < 9; i++) { // Loop through LCD commands
        temp1 = command[i];
        lcd_write();
    }
    flag1 = 1; // Set flag for data mode
    for (int i = 0; msg1[i] != '\0'; i++) { // Loop through message buffer
        temp1 = msg1[i];
        lcd_write();
    }
}

// Function to write data to ports
void port_write() {
    LPC_GPIO0->FIOPIN = temp2 << 23; // write data to GPIO port 0
    if (flag1 == 0) {
        LPC_GPIO0->FIOCLR = 1 << 27; // Clear RS pin
    } else {
        LPC_GPIO0->FIOSET = 1 << 27; // Set RS pin
    }
    LPC_GPIO0->FIOSET = 1 << 28; // Set E pin
    for (j = 0; j < 50; j++); // Delay
    LPC_GPIO0->FIOCLR = 1 << 28; // clear E pin
    for (j = 0; j < 30000; j++); // Delay
}

// Function to write data to LCD
void lcd_write() {
    temp2 = (temp1 >> 4) & 0xF; // Get upper nibble
    port_write(); // Write upper nibble
    if (!(flag1 == 0 && (temp1 == 0x20 || temp1 == 0x30))) { // Check for specific conditions
        temp2 = temp1 & 0xF; // Get lower nibble
        port_write(); // Write lower nibble
    }
}

// Function to rotate motor clockwise
void clock_wise(void) {
    var1 = 0x00000080; // Set rotation direction
    for (m = 0; m < 3; m++) { // Loop for stepping
        LPC_GPIO0->FIOMASK = 0XFFFFFF0F; // Set GPIO mask
        LPC_GPIO0->FIOPIN = var1; // Set GPIO output
        LPC_GPIO0->FIOMASK = 0X00000000; // Clear GPIO mask
        var1 = var1 >> 1; // shift variable
        for (k = 0; k < 60000; k++); // Delay for motor speed variation
    }
}

// Function for keyboard input
void keyboard() {
    //first while loop to check for first key entered
    while (1) {
        int keypressed = 0;
        int br = 0;
        flag1 = 0;
        for (i = 0; i < 9; i++) {
            temp1 = command[i];
            lcd_write();
        }
        flag1 = 1;
        i = 0;
        while (enter_pass[i] != '\0') {
            if (i == 16) {
                flag1 = 0;
                temp1 = 0xC0;
                lcd_write();
            }
            flag1 = 1;
            temp1 = enter_pass[i];
            lcd_write();
            //check keyboard input
            for (row = 0; row < 4; row++) {
                LPC_GPIO2->FIOPIN = 1 << (10 + row);
                for (k = 0; k < 50; k++);
                x = (LPC_GPIO1->FIOPIN >> 23) & 0XF;
                if (x) {
                    if (x == 1)
                        col = 0;
                    else if (x == 2)
                        col = 1;
                    else if (x == 4)
                        col = 2;
                    else if (x == 8)
                        col = 3;
                    sprintf(pass, "%ld", seven_n[row][col]);
                    keypressed = 1;
                    break;
                }
                for (k = 0; k <= 50; k++);
            }
            if (keypressed) {
                br = 1;
                break;
            }
            for (k = 0; k < 500000; k++);
            i++;
        }
        if (br) {
            break;
        }
    }
    flag1 = 0;
    temp1 = 0x01;
    lcd_write();
    for (i = 0; i < 1000; i++);
    i = 0;
    while (pass[i] != '\0') {
        if (i == 16) {
            flag1 = 0;
            temp1 = 0xC0;
            lcd_write();
        }
        flag1 = 1;
        temp1 = 42;
        lcd_write();
        for (k = 0; k < 500000; k++);
        i++;
    }
    pass_len = 1;
    //second while loop to handle rest of the password
    while (1) {
        long int number;
        int keypressed = 0;
        //check keyboard input
        for (row = 0; row < 4; row++) {
            // Note: There appears to be a typo in the original report code here:
            // "6PIO2->FIOPIN=1&(10+row);" is likely meant to be
            // LPC_GPIO2->FIOPIN = 1 << (10 + row);
            // I'm reproducing the code as shown in the report:
            // LPC_GPIO2->FIOPIN=1&(10+row);
            // However, a functional implementation would use the bit shift operation (<<)

            for (k = 0; k < 50; k++);
            x = (LPC_GPIO1->FIOPIN >> 23) & 0XF;
            if (x) {
                if (x == 1)
                    col = 0;
                else if (x == 2)
                    col = 1;
                else if (x == 4)
                    col = 2;
                else if (x == 8)
                    col = 3;
                for (k = 0; k < 5000; k++);
                number = seven_n[row][col];
                //check if enter button pressed
                if (number == 0xf) {
                    keypressed = 1;
                    break;
                }
                //check the number entered
                if (number >= 0 && number <= 9) {
                    sprintf(pass + pass_len, "%ld", number);
                } else if (number >= 0xa && number <= 0xf) {
                    char hexChars[] = "ABCDEF";
                    char hexChar = hexChars[number - 10];
                    sprintf(pass + pass_len, "%c", hexChar);
                }
                pass_len++;
                flag1 = 1;
                temp1 = 42;
                lcd_write();
                if (pass_len == 16) {
                    flag1 = 0;
                    temp1 = 0xC0;
                    lcd_write();
                }
            }
            for (k = 0; k <= 500000; k++);
        }
        if (keypressed) {
            break;
        }
        flag1 = 0;
        temp1 = 0x01;
        lcd_write();
        i = 0;
        while (pass[i] != '\0') {
            if (i == 16) {
                flag1 = 0;
                temp1 = 0x00;
                lcd_write();
            }
            flag1 = 1;
            temp1 = pass[i];
            lcd_write();
            for (k = 0; k < 500000; k++);
            i++;
        }
    }

    //code to handle correct password scenario
    if (strcmp(pass, "1234") == 0) {
        flag1 = 0;
        temp1 = 0x01;
        lcd_write();
        i = 0;
        while (yes[i] != '\0') {
            if (i == 16) {
                flag1 = 0;
                temp1 = 0xC0;
                lcd_write();
            }
            flag1 = 1;
            temp1 = yes[i];
            lcd_write();
            for (k = 0; k < 500000; k++);
            i++;
        }
        for (j = 0; j < 20; j++) // 20 times in Clock wise Rotation
            clock_wise();
        for (k = 0; k < 65000; k++);
        sprintf(msg1, "Headcount is: %d", count);
        display();
        //while loop to handle sensor input and headcount
        while (1) {
            x1 = LPC_GPIO1->FIOPIN & (1 << 15);//first sensor
            x2 = LPC_GPIO1->FIOPIN & (1 << 16);//second sensor
            if (x1 && !x2) { // no entry, no exit
                entrf = 0;
                exitf = 0;
            } else {
                // exit
                if (x1) {
                    exted = 1;
                    exitf--;
                    if (exitf == -1)
                        continue;
                    exitf = 1;
                    if (entred == 1) {
                        entred = 0;
                        count++;
                        LPC_GPIO0->FIOPIN = 0xF << 8;
                        sprintf(msg1, "Headcount is: %d", count);
                        display();
                    } else
                        exted = 1;
                }
                else { // entry
                    if (!x2)
                        continue;
                    exitf = 0;
                    if (entrf == 1)
                        continue;
                    entrf = 1;
                    if (exted == 1) {
                        exted = 0;
                        count--;
                        if (count < 0)
                            count = 0;
                        LPC_GPIO0->FIOPIN = 0xF << 4;
                        sprintf(msg1, "Headcount is: %d", count);
                        display();
                    }
                    else
                        entred = 1;
                }
            }
            for (int j1 = 0; j1 < 100000; j1++);
        }
    }
    //code to handle incorrect password scenario
    else {
        flag1 = 0;
        temp1 = 0x01;
        lcd_write();
        i = 0;
        while (no[i] != '\0') {
            if (i == 16) {
                flag1 = 0;
                temp1 = 0xC0;
                lcd_write();
            }
            flag1 = 1;
            temp1 = no[i];
            lcd_write();
            for (k = 0; k < 800000; k++);
            i++;
        }
        for (k = 0; k < 3; k++) {
            LPC_GPIO0->FIOSET = 1 << 11;
            for (int l = 0; l < 1000000; l++);
            LPC_GPIO0->FIOCLR = 1 << 11;
            for (int l = 0; l < 900000; l++);
        }
    }
}