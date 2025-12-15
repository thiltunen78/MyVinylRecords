//
// CHECK DISPLAY/BOARD CONFIGURATION FROM User_Setup.h !!!
//

#include <TFT_eSPI.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <string>
#include <algorithm>
#include <SD.h>
#include "Records.h"
#include "start_image.h"
#include "bg_image.h"

constexpr uint16_t SDCSPin{4};
constexpr uint16_t LoopDelay{50};
constexpr uint16_t StartDelay{3000};
constexpr uint16_t ConnectionFailedDelay{4000};
constexpr uint16_t ConnectionLoopDelay{500};
constexpr uint16_t ConnectionRetries{20};
constexpr uint16_t MaxArtistLenOnList{39};
constexpr uint16_t MaxAlbumLenOnList{33};
constexpr uint16_t MaxRecordsOnScreen{13};
constexpr uint16_t LeftButtonHighLimit{7};
constexpr uint16_t LeftButtonLowLimit{0};
constexpr uint16_t RightButtonHighLimit{185};
constexpr uint16_t RightButtonLowLimit{182};
constexpr uint16_t UpButtonHighLimit{43};
constexpr uint16_t UpButtonLowLimit{35};
constexpr uint16_t DownButtonHighLimit{102};
constexpr uint16_t DownButtonLowLimit{95};
constexpr uint16_t OkButtonHighLimit{379};
constexpr uint16_t OkButtonLowLimit{376};
constexpr uint16_t SelectionBoxWidth{240};
constexpr uint16_t SelectionBoxHeight{24};
constexpr uint16_t SelectionStep{24};
constexpr uint16_t RecordPageSelectionBoxWidth{240};
constexpr uint16_t RecordPageSelectionBoxHeight{14};
constexpr uint16_t RecordPageSelectionStep{16};
constexpr uint16_t RecordPageSelectionBoxInitialRectY{202};
constexpr uint16_t TotalCountY{285};
constexpr uint16_t ConnectingToWiFiY{283};

enum class RecordPageSelection {
  Exit,
  AddOne,
  AddTwo
};

RecordPageSelection recordPageSelection{RecordPageSelection::Exit};
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "fi.pool.ntp.org", 7200, 60000);
Records *pRecords;
Records twelveRecords("12 Inch Vinyl Records", TFT_ORANGE, TFT_MAROON, "12.txt", 200);
Records tenRecords("10 Inch Vinyl Records", TFT_GREEN, TFT_PURPLE, "10.txt", 50);
Records sevenRecords("7 Inch Vinyl Records", TFT_SKYBLUE, TFT_NAVY, "7.txt", 150);
TFT_eSPI tft = TFT_eSPI();
bool bButtonPressed{false};
bool bShowingRecordPage{false};
int32_t iRectY{0};
int32_t iRectYRecordPage{RecordPageSelectionBoxInitialRectY};
int32_t iSelectedIndex{0};
int32_t iSelectedIndexScreen{0};
std::string strDate{"n/a"};


std::pair<std::string,std::string> readWiFiSettings()
{
  std::string strTemp{""};
  bool bSettingSSID{false};
  bool bSettingPassword{false};
  std::string strSSID{""};
  std::string strPassword{""};
  fs::File file;

  //if wifi.txt file is missing then create example file
  if (!SD.exists("wifi.txt")) {
    file = SD.open("wifi.txt", FILE_WRITE);
    if (file) {    
      file.println("ssid:your_ssid");
      file.println("password:your_password");
    }    
    file.close();
  }  

  file = SD.open("wifi.txt");
  if (file) {
    // read from the file until there's nothing else in it:
    while (file.available()) { 
      char c = file.read();      
      if((c != '\n') && (c != '\r') && (c != ':')) {
        strTemp.push_back(c);      
      }
      else {
        if (strTemp == "ssid") {
          bSettingSSID = true;
          bSettingPassword = false;
          strTemp = "";
        }
        else if (strTemp == "password") {
          bSettingPassword = true;
          bSettingSSID = false;
          strTemp = "";
        }        
        else {
          if (bSettingSSID) {
            strSSID = strTemp;
            strTemp = "";
            bSettingSSID = false;
          }
          else if (bSettingPassword) {
            strPassword = strTemp;
            strTemp = "";
            bSettingPassword = false;
          }
        }
      }
    }
    file.close();
  } 
  return std::make_pair(strSSID, strPassword);
}

std::string convertDate(const std::string &s)
{
  if (s.compare("n/a") == 0)
    return s;

  return s.substr(8, 2) + "." + s.substr(5, 2) + "." + s.substr(0, 4);
}

void setup(void) {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_WHITE); 
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, START_IMAGE_WIDTH, START_IMAGE_HEIGHT, start_image);
  tft.setTextSize(1);
  tft.setCursor(5, ConnectingToWiFiY, 1);
  tft.setTextColor(TFT_WHITE);
  tft.print("Connecting to WiFi.");

  if (!SD.begin(SDCSPin)) {    
    return;
  }
  auto wifiSettings = readWiFiSettings();

  // This is to enable back faster screen updating after SD card operations
  SPI.beginTransaction({40000000, MSBFIRST, SPI_MODE0});

  uint8_t retries{ConnectionRetries};
  WiFi.begin(wifiSettings.first.c_str(), wifiSettings.second.c_str());
  while ((WiFi.status() != WL_CONNECTED) && retries) {
    delay(ConnectionLoopDelay);
    tft.print(".");
    retries--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    tft.setTextColor(TFT_GREEN);
    tft.println("");
    tft.setCursor(5, tft.getCursorY(), 1);
    tft.println("Connected successfully.");

    timeClient.begin();
    timeClient.update();
    time_t rawTime = (time_t)(timeClient.getEpochTime());
    struct tm *timeInfo;
    timeInfo = localtime(&rawTime);
    
    strDate = std::to_string(timeInfo->tm_year + 1900);
    strDate.append("-");
    if ((timeInfo->tm_mon + 1) < 10)
      strDate.append("0");
    strDate.append(std::to_string(timeInfo->tm_mon + 1));
    strDate.append("-");
    if ((timeInfo->tm_mday) < 10)
      strDate.append("0");
    strDate.append(std::to_string(timeInfo->tm_mday));

    tft.setTextColor(TFT_WHITE);
    tft.println("");
    tft.setCursor(5, tft.getCursorY(), 1);
    tft.print("Date fetched: ");
    tft.println(convertDate(strDate).c_str());
  }
  else {
    tft.setTextColor(TFT_RED);    
    tft.println("");
    tft.setCursor(5, tft.getCursorY(), 1);
    tft.println("Connection failed.");
    tft.setCursor(5, tft.getCursorY(), 1);
    tft.println("Date not available.");
    tft.setCursor(5, tft.getCursorY(), 1);
    tft.println("Check WiFi settings from SD card.");
    delay(ConnectionFailedDelay);
  }

  delay(StartDelay);  
  
  //wifi is not needed anymore
  timeClient.end();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);  

  pRecords = &twelveRecords;
  printFirstPage();
}

void showRecordPage(int32_t index)
{
  bShowingRecordPage = true;
  recordPageSelection = RecordPageSelection::Exit;

  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, BG_IMAGE_WIDTH, BG_IMAGE_HEIGHT, bg_image);
  tft.setCursor(5, 4, 4);
  tft.setTextColor(TFT_WHITE);
  tft.println(pRecords->getArtist(index).c_str());
  tft.setCursor(7, tft.getCursorY(), 2);
  tft.println(pRecords->getAlbum(index).c_str());
  tft.println("");
  tft.setTextColor(TFT_ORANGE);  
  tft.setCursor(5, tft.getCursorY(), 4);
  tft.print("Listening count: ");
  tft.println(pRecords->getCount(index).c_str());
  tft.setCursor(7, tft.getCursorY(), 2);
  tft.print("Last played: ");
  tft.println(convertDate(pRecords->getLastPlayedDate(index)).c_str());
  tft.setCursor(7, tft.getCursorY(), 2);
  tft.print("Notes: ");
  tft.println(pRecords->getNotes(index).c_str());
  tft.setCursor(7, RecordPageSelectionBoxInitialRectY - 2, 2);
  tft.println(">> Exit");
  tft.setCursor(7, tft.getCursorY(), 2);
  tft.println(">> Add 1");
  tft.setCursor(7, tft.getCursorY(), 2);
  tft.println(">> Add 2");
  tft.setCursor(7, TotalCountY, 2);
  tft.setTextColor(TFT_RED);
  tft.println("Total listening count of all records:");
  tft.setCursor(7, tft.getCursorY(), 2);
  tft.print(std::to_string(twelveRecords.getTotalCount() + tenRecords.getTotalCount() + sevenRecords.getTotalCount()).c_str());
  tft.print(" (12\": ");
  tft.print(std::to_string(twelveRecords.getTotalCount()).c_str());
  tft.print(", 10\": ");
  tft.print(std::to_string(tenRecords.getTotalCount()).c_str());
  tft.print(", 7\": ");
  tft.print(std::to_string(sevenRecords.getTotalCount()).c_str());
  tft.print(")");

  iRectYRecordPage = RecordPageSelectionBoxInitialRectY;
  tft.drawRoundRect(0, iRectYRecordPage, RecordPageSelectionBoxWidth, RecordPageSelectionBoxHeight, 4, TFT_RED);
}

void printPage(int32_t startIndex)
{
  int16_t albumsOnSreen{MaxRecordsOnScreen};

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(6, 4, 1); 

  if (startIndex < 0)
    startIndex = 0;

  if (startIndex == 0) {     
    tft.fillRoundRect(2, 2, SelectionBoxWidth - 4, SelectionBoxHeight - 4, 4, pRecords->getColor2());
    tft.setTextColor(pRecords->getColor1());
    tft.println(pRecords->getTitle().c_str());
    tft.setCursor(5, tft.getCursorY(), 1);
    tft.print("(");
    tft.print(std::to_string(pRecords->getSize()).c_str());
    tft.print(" kpl)");
    tft.setTextColor(TFT_SILVER);
    tft.print(" Sorted by ");
    tft.println(pRecords->getSortModeString().c_str());
    tft.println("");
    albumsOnSreen--;
  }

  for (int i = startIndex; i < (startIndex + albumsOnSreen); i++) {
    if (pRecords->getSize() == i)
      break;    
    tft.setCursor(5, tft.getCursorY(), 1);
    tft.setTextColor(TFT_WHITE);
    std::string str{pRecords->getArtist(i)};
    if (str.length() > MaxArtistLenOnList) {
      str = str.substr(0,MaxArtistLenOnList - 2);
      str.append("..");
    }    
    tft.println(pRecords->getArtist(i).c_str());
    tft.setCursor(5, tft.getCursorY(), 1);
    tft.setTextColor(TFT_DARKGREY);
    str = pRecords->getAlbum(i);
    if (str.length() > MaxAlbumLenOnList) {
      str = str.substr(0,MaxAlbumLenOnList - 2);
      str.append("..");
    }
    str.append(" (");
    tft.print(str.c_str());
    tft.setTextColor(pRecords->getColor1());
    tft.print(pRecords->getCount(i).c_str());
    tft.setTextColor(TFT_DARKGREY);
    tft.println(")");
    tft.println("");
  }; 
}

void readKeyboard()
{
  int32_t iButtonValue{analogRead(A0)};

  Serial.println(iButtonValue);

  if ((iButtonValue <= LeftButtonHighLimit) && (iButtonValue >= LeftButtonLowLimit)) {    
    processLeftButton();
  }
  else if ((iButtonValue <= UpButtonHighLimit) && (iButtonValue >= UpButtonLowLimit)) {
    processUpButton();
  }
  else if ((iButtonValue <= DownButtonHighLimit) && (iButtonValue >= DownButtonLowLimit)) {    
    processDownButton();
  }
  else if ((iButtonValue <= RightButtonHighLimit) && (iButtonValue >= RightButtonLowLimit)) {    
    processRightButton(); 
  }
  else if ((iButtonValue <= OkButtonHighLimit) && (iButtonValue >= OkButtonLowLimit)) {    
    processOkButton();
  }
  else {
    bButtonPressed = false;
  }
}

void processUpButton()
{
  if (bButtonPressed)
    return;  
  bButtonPressed = true;

  if (bShowingRecordPage) {
    if (recordPageSelection != RecordPageSelection::Exit) {
      drawRecordPageRect(true);
    }
    if (recordPageSelection == RecordPageSelection::AddTwo) {
      recordPageSelection = RecordPageSelection::AddOne;
    }
    else if (recordPageSelection == RecordPageSelection::AddOne) {
      recordPageSelection = RecordPageSelection::Exit;
    }
  }
  else if (iSelectedIndex > 0) {
    iSelectedIndex--;
    if (iSelectedIndexScreen > 0) {     
      iSelectedIndexScreen--;
      drawRect(iSelectedIndexScreen);     
    }
    else {        
      iSelectedIndexScreen = MaxRecordsOnScreen - 1;        
      printPage(iSelectedIndex - MaxRecordsOnScreen);
      drawRect(iSelectedIndexScreen);
    }
  }
  else {
    pRecords->sort();
    printFirstPage();
  }
}

void processDownButton()
{
  if (bButtonPressed)
    return;
  bButtonPressed = true;

  if (bShowingRecordPage) {
    if (recordPageSelection != RecordPageSelection::AddTwo) {
      drawRecordPageRect(false);
    }
    if (recordPageSelection == RecordPageSelection::Exit) {
      recordPageSelection = RecordPageSelection::AddOne;
    }
    else if (recordPageSelection == RecordPageSelection::AddOne) {
      recordPageSelection = RecordPageSelection::AddTwo;
    }
  }
  else if (iSelectedIndex < pRecords->getSize()) {
    iSelectedIndex++;
    if (iSelectedIndexScreen < (MaxRecordsOnScreen - 1)) {     
      iSelectedIndexScreen++;
      drawRect(iSelectedIndexScreen);
    }
    else {
      iSelectedIndexScreen = 0;        
      printPage(iSelectedIndex - 1);
      drawRect(iSelectedIndexScreen);
    }
  }
}

void processLeftButton()
{
  if (bButtonPressed)
    return;  
  bButtonPressed = true;

  if (bShowingRecordPage)
    return;

  if (iSelectedIndex > (MaxRecordsOnScreen - 1)) {
    iSelectedIndex -= (MaxRecordsOnScreen + iSelectedIndexScreen);
    iSelectedIndexScreen = 0;
    printPage(iSelectedIndex - 1);
    drawRect(iSelectedIndexScreen);
  }
}

void processRightButton()
{
  if (bButtonPressed)
    return;  
  bButtonPressed = true;

  if (bShowingRecordPage)
    return;

  if ((iSelectedIndex + (MaxRecordsOnScreen - iSelectedIndexScreen)) <= pRecords->getSize()) {
    iSelectedIndex += (MaxRecordsOnScreen - iSelectedIndexScreen);
    iSelectedIndexScreen = 0;
    printPage(iSelectedIndex - 1);
    drawRect(iSelectedIndexScreen);
  }
}

void processOkButton()
{
  if (bButtonPressed)
    return;  
  bButtonPressed = true;

  if (bShowingRecordPage) {
    if (recordPageSelection == RecordPageSelection::AddOne) {
      pRecords->increaseCount(iSelectedIndex - 1, 1, strDate);          
    }
    else if (recordPageSelection == RecordPageSelection::AddTwo) {
      pRecords->increaseCount(iSelectedIndex - 1, 2, strDate);        
    }

    bShowingRecordPage = false;

    if ((recordPageSelection != RecordPageSelection::Exit) && (pRecords->getSortMode() != SortMode::Name)) {
      printFirstPage();
    }
    else {
      printPage(iSelectedIndex - iSelectedIndexScreen - 1);
      drawRect(iSelectedIndexScreen);
    }
  }
  else if (iSelectedIndex > 0) {
    showRecordPage(iSelectedIndex -1 );
  }
  else {
    if (pRecords == &twelveRecords) {
      pRecords = &tenRecords;
    }
    else if (pRecords == &tenRecords) {
      pRecords = &sevenRecords;
    }
    else if (pRecords == &sevenRecords) {
      pRecords = &twelveRecords;
    }

    printFirstPage();
  }
}

void printFirstPage() 
{
  printPage(0);
  drawRect(0);
  iSelectedIndexScreen = 0;
  iSelectedIndex = 0;
}

void drawRect(int32_t index) 
{
  tft.drawRoundRect(0, iRectY, SelectionBoxWidth, SelectionBoxHeight, 4, TFT_BLACK);
  iRectY = index * SelectionStep;
  tft.drawRoundRect(0, iRectY, SelectionBoxWidth, SelectionBoxHeight, 4, pRecords->getColor1());
}

void drawRecordPageRect(bool up) 
{
  tft.drawRoundRect(0, iRectYRecordPage, RecordPageSelectionBoxWidth, RecordPageSelectionBoxHeight, 4, TFT_BLACK);  
  iRectYRecordPage = up ? (iRectYRecordPage - RecordPageSelectionStep) : (iRectYRecordPage + RecordPageSelectionStep);
  tft.drawRoundRect(0, iRectYRecordPage, RecordPageSelectionBoxWidth, RecordPageSelectionBoxHeight, 4, TFT_RED);
}

void loop()
{  
  readKeyboard();
  delay(LoopDelay);
}
