# My Vinyl Records
ESP8266 microcontroller project to track my vinyl record listening times.

I needed a separate device to track my record listening times so I made one by using ESP8266 microcontroller, 3.2 inch lcd module with SD card reader, 5 button keyboard module, lipo battery and charging circuit. I put everything neatly between two pieces of plexiglass.

## Features:
- On startup device will fetch current date using wifi. Date is later added to record entry as its listening counts are increased. Wifi settings are stored to file in SD card. 
- Three record list views. One for 12 inch records, one for 10 inch records and one for 7 inch records. One record entry in the list contains information about the artist, album, and number of times it has been listened to. Lists can be changed by pressing the select button on the top header. Lists are stored to SD card as separated txt files.
- List views can be sorted based on listening count, name or last listening date. Sort mode can be changed by pressing the up button on the top header.
- Navigation in the list is done using the up and down buttons. Using the right or left button, it is possible to scroll through a page at a time.
- When a record is selected from the list a new view is opened to show information from the record entry. For example listening count and last played date. In this view it is possible to increase listening count with one or two counts. When count is icreased the data is saved to the SD card.