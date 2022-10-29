# TTN-Mapper-Cubecell-GPS
This repositorty expains the procedure to connect the Heltec Cubcell GPS HTCC-AB02S with the TTN Mapper


1. Create a new application on the things network say "TTN Mapper"
       * Add a new end-device to the application. Use the following:
       * Select the end device in the LoRaWAN Device Repository
       * End Device Brand: Heltech AutoMation
       * HTCC-AB02S (Class A)
       * Hardware Version: Unknown
       * Firmware version: 1.0
       * Profile(Region): Set according to your region. Mine was IN_865_867 as I am from India.
       * Select Frequency Plan: Set according to your region
       * Set JOINEUI: You can sent it all to zero or any random code. This will be used in the next step
       * Generate DEVEUI and APPKEY
       * Set some random "End Device ID" and click Register End Device button.
      
2. Add the payload formatter in your TTN application created in the Step 1 as follows:
- Go to Overview -> Payload formatter -> Uplink 
- Select "Formatter Type" as Custom Javascript formatter and paste the contenct of "PayLoadFormatter.txt"
- Click Save Changes

3. Add the webhook in your TTN application as follows:
- Go to Integration -> Webhooks -> Click Add Webhook
- Select Webhook template "TTN Mapper"
- Set "Webhook ID" and email address to create the webhook
- The webhook will be active in a couple of miniutes

4. Upload sketch to Heltec module via Arduino IDE
- Update devEui[], appEui[], and appKey[] in the sketch "CubeCellTTNMapper.ino" received from in Step 1.
- Select the correct REGION frequency and NETMODE as "OTAA" in the Arduino IDE 'Tool' dropdown menu. 
- Upload sketch to the Heltec GPS board.
