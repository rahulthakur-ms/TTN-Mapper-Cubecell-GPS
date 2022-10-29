# TTN-Mapper-Cubecell-GPS
This repositorty expains the procedure to connect the Heltec Cubcell GPS HTCC-AB02S with the TTN Mapper


Step 1:
Create a new application on the things network say "TTN Mapper"
Add a new end-device to the application. Use the following:
      A. Select the end device in the LoRaWAN Device Repository
      B. End Device Brand: Heltech AutoMation
      C. HTCC-AB02S (Class A)
      D. Hardware Version: Unknown
      E. Firmware version: 1.0
      F: Profile(Region): Set according to your region. Mine was IN_865_867 as I am from India.
      G. Select Frequency Plan: Set according to your region
      H. Set JOINEUI: You can sent it all to zero or any random code. This will be used in the next step
      I. Generate DEVEUI and APPKEY
      J. Set some random "End Device ID" and click Register End Device button.
      
Step 2:
Add the payload formatter in your application created in the Step 1 as follows:
Go to Overview -> Payload formatter -> Uplink 
Select "Formatter Type" as Custom Javascript formatter and paste the contenct of "PayLoadFormatter.txt"
Click Save Changes



Step 2: 
Update devEui[], appEui[], and appKey[] in the sketch "CubeCellTTNMapper.ino" received from in Step 1.
Select the correct REGION frequency and NETMODE as "OTAA" in the Arduino IDE 'Tool' dropdown menu. 
Upload sketch to the Heltec GPS board.