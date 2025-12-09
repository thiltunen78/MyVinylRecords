#ifndef RECORDS_H
#define RECORDS_H

#include <string>
#include <vector>
#include <Arduino.h>

enum class SortMode {
  Count,
  Name,
  Date
};

class Records 
{
public:
  struct Data {
    std::string strArtist;
    std::string strAlbum;
    uint16_t uCount;
    std::string strLastPlayedDate;
    std::string strNotes;
  };

  Records(std::string strTitle, int32_t iColor1, int32_t iColor2, std::string strFile, uint16_t uReserve);
  std::string getArtist(int32_t index);
  std::string getAlbum(int32_t index);
  std::string getCount(int32_t index);
  std::string getLastPlayedDate(int32_t index);
  std::string getNotes(int32_t index);
  uint16_t getTotalCount();
  std::size_t getSize();
  std::string getTitle();
  int32_t getColor1();
  int32_t getColor2();
  void sort();
  SortMode getSortMode();
  std::string getSortModeString();
  void increaseCount(int32_t index, int32_t amount, std::string &date);

private:
  void readFile();
  void writeFile(); 

  std::vector<Data> m_vectData;
  std::string m_strTitle;
  int32_t m_iColor1;
  int32_t m_iColor2;
  std::string m_strFile;
  SortMode m_sortMode;
  uint16_t m_uTotalCount;
};

#endif