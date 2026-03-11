This project is a simple application that gets local weather data and puts it through an algorithm to generate a drying score that determines how suitable the climate
is to dry your clothes outside. The idea for this came to me when the weather was dry but mild and I wasnt sure if there was a benefit to drying the clothes outside
compared to inside. 
This should give the user some insight to enable them to make a decision.
The project includes 3d print files aswell as an image of the device.

The device features a standard Oled display (SSD 1306) aswell as a push button.
The code shows which GPIO to connect the push button to.

The SSD1306 uses the standar SDA and SCL pins relevent to your device.
The microcontroller used was the ESP32 devkit-c so check with your micro controller which to use.

To connect to your wifi network there are 2 options!
1. Create a secrets file which contains your SSID, password, longitude, latitude and city. (arduino_secrets.h)
   SetusingAtHome to true in the .ino file. This will use hardcoded data.

2. Alternatively, download the ESP BLE provisioning app from the app store on your smartphone. Plug in the esp32 to your arduino IDE and upon startup it will print a QR code,
   scan this QR code with the provisioning app and follow the instructions. This will get you on the wifi network and the credentials will be stored in the devices NVS
   so it will auto connect next time you switch it on.

Note that the provisioning method will only give you approximate coordinates or even significantly incorrect coordinates depending on your network configuration, so for the
sake of efficacy, going the hardcoded route may be better.
The push button can be held down for 5 seconds, this reboots the device and erases the stored wifi credentials from the NVS, this can be used if your wifi network changes
etc.

Email me at edwinthomspson98@gmail.com if you have questions :)
