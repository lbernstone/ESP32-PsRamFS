#include "FS.h"
#include "PSRamFS.h"
#include "pfs.h"

//#include "test/test_pfs.h"

// You don't really need to format PSRamFS unless previously used
#define FORMAT_PSRAMFS true
#define UNIT_TESTS



#ifdef UNIT_TESTS

#include "tests/vfs_pfs_tests.h"

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
/*
    if(!PSRamFS.begin()){
      log_e("PSRamFS Mount Failed, halting");
      return;
    }


    log_n("Total space: %10u", PSRamFS.totalBytes());
    log_n("Free space: %10u", PSRamFS.freeBytes());
*/
    delay(2000); // service delay
    UNITY_BEGIN();

    RUN_TEST(test_setup_teardown);
    RUN_TEST(test_can_format_mounted_partition);
    RUN_TEST(test_ftell);
    RUN_TEST(test_stat_fstat);
/*
    RUN_TEST(test_string_concat);
    RUN_TEST(test_string_substring);
    RUN_TEST(test_string_index_of);
    RUN_TEST(test_string_equal_ignore_case);
    RUN_TEST(test_string_to_upper_case);
    RUN_TEST(test_string_replace);
*/
    UNITY_END(); // stop unit testing
}

void loop()
{
}

#else




void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
{
  delay(100);
  log_n("Listing directory: %s", dirname);

  struct PSRAMFILE
  {
    int file_id;
    char * name;
    char * bytes;
    unsigned long size;
    unsigned long memsize;
    unsigned long index;
  };

  PSRAMFILE ** myFiles = (PSRAMFILE **)PSRamFS.getFiles();

  if( myFiles != NULL ) {
    size_t myFilesCount = pfs_get_max_items();
    if( myFilesCount > 0 ) {
      for( int i=0; i<myFilesCount; i++ ) {
        if( myFiles[i]->name != NULL ) {
          log_w("Entity #%d : %s / %d / %d", i, myFiles[i]->name, myFiles[i]->size, myFiles[i]->index );
        }
      }
    } else {
      log_w("Directory empty");
    }
  }
/*
  File root = fs.open(dirname);
  if(!root){
    log_n("- failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    log_n(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      log_n("  DIR : %s",  file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      log_n("  FILE: %s\t%d", file.name(), file.size());
    }
    file = root.openNextFile();
  }
  */
  log_n("Listing done\n");
}



void readFile(fs::FS &fs, const char * path)
{
  delay(100);
  ESP_LOGD(TAG, "Will open file for reading: %s", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    ESP_LOGE(TAG, "Failed to open file for reading : %s", path);
    return;
  }

  if( !file.available() ) {
    ESP_LOGE(TAG, "CANNOT read from file: %s", path);
  } else {
    ESP_LOGD(TAG, "Will read from file: %s", path);

    delay(100);
    Serial.println();

    int32_t lastPosition = -1;
    while( file.available() ) {
      size_t position = file.position();
      char a = file.read();
      Serial.write( a );
      if( lastPosition == position ) { // uh-oh
        Serial.println("Halting");
        while(1);
        break;
      } else {
        lastPosition = position;
      }
    }
  }
  Serial.println("\n");
  file.close();
  ESP_LOGD(TAG, "Read done: %s", path);
}



void writeFile(fs::FS &fs, const char * path, const char * message)
{
  delay(100);

  ESP_LOGD(TAG, "Will truncate %s using %s mode", path, FILE_WRITE );

  File file = fs.open(path, FILE_WRITE);

  if(!file){
    ESP_LOGE(TAG, "Failed to open file %s for writing", path);
    return;
  } else {
    ESP_LOGD(TAG, "Truncated file: %s", path);
  }
  if( file.write( (const uint8_t*)message, strlen(message)+1 ) ) {
    ESP_LOGD(TAG, "- data written");
  } else {
    ESP_LOGE(TAG, "- write failed\n");
  }
  file.close();
}



void appendFile(fs::FS &fs, const char * path, const char * message)
{
  delay(100);

  log_n("Will append %s using %s mode", path, FILE_APPEND );
  //log_n("Appending to file: %s", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    log_e("- failed to open file for appending");
    return;
  }
  if( file.write( (const uint8_t*)message, strlen(message)+1 ) ){
    log_n("- message appended");
  } else {
    log_e("- append failed\n");
  }
  file.close();
}



void renameFile(fs::FS &fs, const char * path1, const char * path2)
{
  delay(100);
  log_n("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    log_n("- file renamed");
  } else {
    log_e("- rename failed");
  }
}



void deleteFile(fs::FS &fs, const char * path)
{
  delay(100);
  log_n("Deleting file: %s\r\n", path);
  if(fs.remove(path)){
    log_n("- file deleted");
  } else {
    log_e("- delete failed");
  }
}



void testFileIO(fs::FS &fs, const char * path)
{
  delay(100);
  log_n("Testing file I/O with %s\r\n", path);

  static uint8_t buf[512];
  size_t len = 0;
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    log_e("- failed to open file for writing");
    return;
  }

  size_t i;
  log_n("- writing" );
  uint32_t start = millis();
  for(i=0; i<1024; i++){
    if ((i & 0x001F) == 0x001F){
      //Serial.printf("\n[%d]", i);
      Serial.print(".");
    }
    file.write(buf, 512);
  }
  Serial.println("");
  uint32_t end = millis() - start;
  log_n(" - %u bytes written in %u ms\r\n", 1024 * 512, end);
  //Serial.print(".");
  file.close();


  File file2 = fs.open(path);
  start = millis();
  end = start;
  i = 0;
  if(file2 && !file2.isDirectory()){
    len = file2.size();
    size_t flen = len;
    start = millis();
    log_n("- reading" );
    while(len){
      size_t toRead = len;
      if(toRead > 512){
        toRead = 512;
      }
      file2.read(buf, toRead);
      if ((i++ & 0x001F) == 0x001F){
        Serial.print(".");
      }
      len -= toRead;
    }
    Serial.println("");
    end = millis() - start;
    log_n("- %u bytes read in %u ms\r\n", flen, end);
    file2.close();
  } else {
    log_n("- failed to open file for reading");
  }
}



void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  if(!PSRamFS.begin()){
    log_e("PSRamFS Mount Failed");
    return;
  }


  log_n("Total space: %10u", PSRamFS.totalBytes());
  log_n("Free space: %10u", PSRamFS.freeBytes());

  listDir(PSRamFS, "/", 0);

  writeFile(PSRamFS, "/blah.txt", "BLAH !");
  writeFile(PSRamFS, "/oops.ico", "???????????????");

  listDir(PSRamFS, "/", 0);

  writeFile(PSRamFS, "/hello.txt", "Hello ");
  appendFile(PSRamFS, "/hello.txt", "World!");
  readFile(PSRamFS, "/hello.txt");

  listDir(PSRamFS, "/", 0);

  renameFile(PSRamFS, "/hello.txt", "/foo.txt");
  readFile(PSRamFS, "/foo.txt");
  deleteFile(PSRamFS, "/foo.txt");

  listDir(PSRamFS, "/", 0);

  testFileIO(PSRamFS, "/test.txt");
  log_n("Free space: %10u\n", PSRamFS.freeBytes());

  listDir(PSRamFS, "/", 0);

  deleteFile(PSRamFS, "/test.txt");

  listDir(PSRamFS, "/", 0);

  log_n( "Test complete" );

}


void loop()
{

}


#endif
