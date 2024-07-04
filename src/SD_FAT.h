#define SECTORPERFAT 9    //founded FAT parameter for this 

uint32_t startPartition;
uint32_t startRoot;
bool SD_init();
void SD_command(uint8_t, uint32_t, uint8_t);
void SD_CMD8();
uint32_t readOCR();
uint8_t SD_readRes1();
void SD_readRes7(uint8_t*);
void SD_powerUpSeq();
uint8_t SD_sendCommand_R1(uint8_t, uint32_t, uint8_t);
void readBlock(uint32_t, uint8_t*);
void writeBlock(uint32_t, uint8_t*);
void ListRootDir(uint8_t*);
uint32_t startOfRoot(uint32_t);
uint32_t startOfFirstPartition();
void printFile(uint16_t, uint32_t, uint32_t, uint32_t);
void readFileusingEntryNumber(uint16_t);
void createFile(char*);
void writeFile(char*, char*);
void clusterConnect(uint16_t, uint16_t);
void reverse(char*, int);
int intToStr(uint16_t, char[], int);
void ftoa(float, char*);
// CODE FOR SDCARD
bool SD_init(){
  masterInit();
  SD_powerUpSeq();
  if(SD_sendCommand_R1(0,0,0b10010100) & 1) return 0;
  return 1;


}

void SD_command(uint8_t cmd, uint32_t arg, uint8_t crc)
{
      spitransfer(cmd|0x40);
    spitransfer((uint8_t)(arg >> 24));
    spitransfer((uint8_t)(arg >> 16));
    spitransfer((uint8_t)(arg >> 8));
    spitransfer((uint8_t)(arg));
    spitransfer(crc|0x01);
}
void SD_CMD8()
{
    uint8_t res[5];
    spitransfer(0xFF);
    SSlow();
    spitransfer(0xFF);
  SD_command(8, (1<<8)|(0b10101010), 0b10000110);
    SD_readRes7(res);
    spitransfer(0xFF);
    SShigh();
    spitransfer(0xFF);
}

uint32_t readOCR(){
  uint8_t res1;
  uint32_t ocr;
  ocr = 0;
  spitransfer(0xFF);
  SSlow();
  spitransfer(0xFF);
  SD_command(58, 0, 0);//CMD58, stuff arg, dummy crc
  res1 = SD_readRes1();
  ocr |= ((uint32_t)(spitransfer(0xFF)))<<24;
  ocr |= ((uint32_t)(spitransfer(0xFF)))<<16;
  ocr |= ((uint32_t)(spitransfer(0xFF)))<<8;
  ocr |= ((uint32_t)(spitransfer(0xFF)));

  return ocr;

}
uint8_t SD_readRes1()
{
    uint8_t i = 0, res1;
    while((res1 = spitransfer(0xFF)) == 0xFF)
    {
        i++;
        if(i > 8) break;
    }
    return res1;
}
void SD_readRes7(uint8_t *res)
{
    // read response 1 in R7
    res[0] = SD_readRes1();

    // if error reading R1, return
    if(res[0] > 1) return;

    // read remaining bytes
    res[1] = spitransfer(0xFF);
    res[2] = spitransfer(0xFF);
    res[3] = spitransfer(0xFF);
    res[4] = spitransfer(0xFF);
}
void SD_powerUpSeq()
{
    SShigh();
    delay(1);
    for(uint8_t i = 0; i < 10; i++)
        spitransfer(0xFF);
    SShigh();
    spitransfer(0xFF);
}

uint8_t SD_sendCommand_R1(uint8_t cmd, uint32_t arg, uint8_t crc){
    spitransfer(0xFF);
    SSlow();
    spitransfer(0xFF);
    SD_command(cmd, arg, crc);
    uint8_t res1 = SD_readRes1();
    spitransfer(0xFF);
    SShigh();
    spitransfer(0xFF);
    return res1;

}

void readBlock(uint32_t address, uint8_t *buffer){
    spitransfer(0xFF);
    SSlow();
    spitransfer(0xFF);

    SD_command(17, address, 0);
    uint8_t res1 = SD_readRes1();
    res1=0xFF;
    while((res1=spitransfer(0xFF))==0xFF);
    if(res1==0xFE){
      for(int i=0;i<512;i++){
        buffer[i] = spitransfer(0xFF);
      }
      spitransfer(0xFF);spitransfer(0xFF); //for CRC
    }

    spitransfer(0xFF);
    SShigh();
    spitransfer(0xFF);
}
void writeBlock(uint32_t address, uint8_t *buffer){
    spitransfer(0xFF);
    SSlow();
    spitransfer(0xFF);
    SD_command(24, address, 0);
    uint8_t res1 = SD_readRes1();
    if(res1==0){
      spitransfer(0xFE);
      for(int i=0;i<512;i++) spitransfer(buffer[i]);
    }
    uint8_t readAttempts = 0;
    while(++readAttempts != 3000)
        if((res1 = spitransfer(0xFF)) != 0xFF) {break;}
    spitransfer(0xFF);
    SShigh();
    spitransfer(0xFF);
}

void ListRootDir(uint8_t *address){
  uint8_t buffer[512];
  readBlock(address, buffer);
  Serial.println("\n*************ROOT DIR*********");
  Serial.println("\nFILENUM-FILENAME-SIZE-StartCluster");
  char name[9] = {0};
  char extension[4] = {0};
  uint32_t size = 0;
  uint16_t startcluster=0;
  uint8_t count = 0;
  uint16_t j=0;

  while((buffer[j])!=0){
    for(int i=0;i<8;i++){if(buffer[i+j]==0x20) continue; name[i] = buffer[i+j];}
    j+=8;  //8bytes for file name
    for(int i=0;i<3;i++){if(buffer[i+j]==0x20) continue; extension[i] = buffer[i+j];}
    j+=18;   //3bytes for extension and some bytes of other information
    startcluster = (((uint16_t)buffer[1+j])<<8)|(((uint16_t)buffer[0+j]));
    j+=2;
    size = (((uint32_t)buffer[3+j])<<24)|(((uint32_t)buffer[2+j])<<16)|(((uint32_t)buffer[1+j])<<8)|(((uint32_t)buffer[0+j]));
    j += 4;  //4bytes of file size
    Serial.print(count+1);
    Serial.print(" - ");
    Serial.print(name);
    Serial.print(".");
    Serial.print(extension);
    Serial.print(" - ");
    Serial.print(size);
    Serial.println(" Bytes");
    // Serial.println(startcluster);
    count++;
    if(count==512) break; //maximum no of entries in root is 512
    if(count%16==0){      //only 16 entries possible in 512 bytes
        readBlock(++address, buffer);j=0;
    }
  }
}
uint32_t startOfRoot(uint32_t startOfFirstPartition){
  uint8_t buffer[512] ={0};
  readBlock(startOfFirstPartition, buffer);
  // uint16_t bytespersector = (((uint16_t)buffer[12])<<8) | buffer[11];
  // uint8_t sectorspercluster = buffer[13];
  // uint16_t reservedSector = (((uint16_t)buffer[0x0F])<<8)|buffer[0xE];
  // uint16_t maxRootEntries = (((uint16_t)buffer[0x12])<<8)|buffer[0x11];
  uint8_t NumberofFAT = buffer[16];
  uint16_t sectorPerFAT = (((uint16_t)buffer[0x17])<<8)|buffer[0x16];
  return startOfFirstPartition+1+(NumberofFAT*sectorPerFAT);
}
uint32_t startOfFirstPartition(){
  uint8_t buffer[512] ={0};
  readBlock(0, buffer);
  if((buffer[511]!=0xAA) || (buffer[510]!=0x55)){
    Serial.println("No MBR");
    return 0;
  }
  if(buffer[446+4]!=0x04){
    Serial.println("No FAT16");
    return 0;
  }
  return (((uint32_t)buffer[457])<<24)|(((uint32_t)buffer[456])<<16)|(((uint32_t)buffer[455])<<8)|(((uint32_t)buffer[454]));
}
void printFile(uint16_t cluster, uint32_t size, uint32_t dataField, uint32_t FAT){
  uint8_t buffer[512], s;
  Serial.print("\n");
  if(size==0){Serial.println("[-]EMPTY FILE");}
  while(size!=0){
    readBlock(dataField+cluster-2, buffer);
  for(uint16_t i=0;(i<512)&&size;i++){
    Serial.print((char)buffer[i]);
    size--;
  }
  if(size){
    s=cluster/512;
    cluster = cluster%512;
    readBlock(FAT+s, buffer);
    cluster = (buffer[(cluster*2)+1] <<8)|  buffer[(cluster*2)];
    if(cluster==0xFFFF) break;
  }
  }
}
// void readFile(char *filename){
//   uint8_t buffer[512] = {1};
//   uint16_t i = 0;
//   uint16_t j = 0;
//   uint8_t l=0;
//   uint8_t temp=0;
//   bool found=1;
//   readBlock(startRoot+j, buffer);
//   while(buffer[i+l]!=0 && j<=32){ //32 tells that max sector for Root is 32  which is 512 entries
//   found = 1;

//   for(uint8_t k=0;(k<8)&&(filename[k]!='.');k++){if(buffer[k+l]!=filename[k]){ found=0;break;}}

//   if(found==1){

//     for(temp=0;(temp<8) && (filename[temp]!='.');temp++);
//     temp++;
//     for(uint8_t k=0;k<3;k++){if(buffer[8+k+l]!=filename[k+temp]){ found=0;break;}}
//     if(found=1) break;
//   }

//   if(found==0) l+=32;

//   if(l>=512){j++;l=0;readBlock(startRoot+j, buffer);}
//   }
//   Serial.print("the value of l: ");
//   Serial.println(l, DEC);
//   uint16_t startcluster = (((uint16_t)buffer[l+1+26])<<8)|(((uint16_t)buffer[l+26]));
//   uint32_t size = (((uint32_t)buffer[l+3+28])<<24)|(((uint32_t)buffer[2+28+l])<<16)|(((uint32_t)buffer[1+28+l])<<8)|(((uint32_t)buffer[0+28+l]));
//   printFile(startcluster, size,startRoot+32, startPartition+1);
// }
void readFileusingEntryNumber(uint16_t l){
  Serial.println("***FILE CONTENT***");
    uint8_t buffer[512] = {1};
    l--;
  uint8_t k = l/16;
  l = l%16;
  l*=32;
  readBlock(startRoot+k, buffer);
  uint16_t startcluster = (((uint16_t)buffer[l+1+26])<<8)|(((uint16_t)buffer[l+26]));
  uint32_t size = (((uint32_t)buffer[l+3+28])<<24)|(((uint32_t)buffer[2+28+l])<<16)|(((uint32_t)buffer[1+28+l])<<8)|(((uint32_t)buffer[0+28+l]));
  printFile(startcluster, size,startRoot+32, startPartition+1);
}

void createFile(char *filename){
  volatile uint8_t buffer[512];
  volatile uint8_t j=0;
  volatile uint16_t k=0;
  readBlock(startPartition+1, buffer);  //finding free cluster
  while((buffer[k]!=0 || buffer[k+1]!=0) && j<=SECTORPERFAT){

    for(k=0;k<511;k+=2){
    if(buffer[k]==0 && buffer[k+1]==0) break;
    }
    if(buffer[k]==0 && buffer[k+1]==0) break;
    else{
      j+=1;readBlock(startPartition+1+j, buffer);k=0;
    }
  }
  buffer[k]==0xFF;buffer[k+1]=0xFF;
  writeBlock(startPartition+1+j, buffer);
  uint16_t startcluster=((j*512)+k)/2;
  readBlock(startRoot, buffer);
  
  uint16_t l=0;
  j=0;
  while(buffer[l]!=0 && j<=32){
    l+=32;
    if(l>=512){l=0;readBlock(startRoot+j,buffer);j++;}
  }
  if(buffer[l]==0){
    for(uint8_t k=0;k<8;k++){buffer[l+k]=filename[k];};
    for(uint8_t k=0;k<3;k++){buffer[l+8+k] = filename[8+1+k];}
    buffer[l+26] = startcluster;
    buffer[l+27] = startcluster>>8;
    for(uint8_t k=0;k<4;k++) buffer[l+28+k] = 0;  //size as 0
    writeBlock(startRoot+j, buffer);
  }
  
}

void writeFile(char *filename, char *content){
  uint8_t buffer[512] = {1};
  uint16_t i = 0;
  uint16_t a = 0;
  uint16_t l=0;
  uint8_t temp=0;
  bool found=1;
  readBlock(startRoot+a, buffer);
  while(buffer[i+l]!=0 && a<=32){ //32 tells that max sector for Root is 32  which is 512 entries
  found = 1;
  for(uint8_t k=0;(k<8)&&(filename[k]!='.');k++){if(buffer[k+l]!=filename[k]){ found=0;break;}}

  if(found==1){

    for(temp=0;(temp<8) && (filename[temp]!='.');temp++);
    temp++;
    for(uint8_t k=0;k<3;k++){if(buffer[8+k+l]!=filename[k+temp]){ found=0;break;}}
    if(found=1) break;
  }

  if(found==0) l+=32;

  if(l>=512){a++;l=0;readBlock(startRoot+a, buffer);}
  }

  uint16_t startcluster = (((uint16_t)buffer[l+1+26])<<8)|(((uint16_t)buffer[l+26]));
  uint32_t size = (((uint32_t)buffer[l+3+28])<<24)|(((uint32_t)buffer[2+28+l])<<16)|(((uint32_t)buffer[1+28+l])<<8)|(((uint32_t)buffer[0+28+l]));
  uint16_t FATsector=0;

  while(size>512){
    FATsector=startcluster/256;
    startcluster = startcluster%256;
    readBlock(startPartition+1+FATsector, buffer);
    startcluster = (buffer[(startcluster*2)+1] <<8)|  buffer[(startcluster*2)];
    if(startcluster==0xFFFF) break;
    size-=512;
  }

  readBlock(startRoot+32+startcluster-2, buffer);
  uint32_t writeSize;
  for(writeSize=0;content[writeSize]!=0;writeSize++){
    uint16_t i=0;
    if((size+i)<512){
      buffer[size+writeSize] = content[writeSize];
      i++;
      }
    else{

        writeBlock(startRoot+32+startcluster-2, buffer);
        uint16_t j=0;
        uint16_t k=0;
        readBlock(startPartition+1, buffer);  //finding free cluster
        while((buffer[k]!=0 || buffer[k+1]!=0) && j<=SECTORPERFAT){

          for(k=0;k<511;k+=2){
            if(buffer[k]==0 && buffer[k+1]==0) break;
          }
          if(buffer[k]==0 && buffer[k+1]==0) break;
          else{
            j+=1;readBlock(startPartition+1+j, buffer);k=0;
          }
        }
        buffer[k]==0xFF;buffer[k+1]=0xFF;
        writeBlock(startPartition+1+j, buffer);
        uint16_t newcluster=((j*512)+k)/2;
        clusterConnect(startcluster, newcluster);
        startcluster = newcluster;newcluster=0;
        size=0;
    }
    writeBlock(startRoot+32+startcluster-2, buffer);
  }

  readBlock(startRoot+a, buffer);
  size = (((uint32_t)buffer[l+3+28])<<24)|(((uint32_t)buffer[2+28+l])<<16)|(((uint32_t)buffer[1+28+l])<<8)|(((uint32_t)buffer[0+28+l]));
  size += writeSize;
  buffer[l+28+0] = size&0x000000FF;
  buffer[l+28+1] = (size >> 8)&0x000000FF;
  buffer[l+28+2] = (size>>16)&0x000000FF;
  buffer[l+28+3] = (size >> 24)&0x000000FF;
  writeBlock(startRoot+a, buffer);


}
void clusterConnect(uint16_t from, uint16_t to){
  char buffer[512];
  from*=2;
  uint8_t FATsector = from/256;
  from = from%256;
  readBlock(startPartition+1+FATsector, buffer);
  buffer[(from*2)+1] = to&0x00FF;
  buffer[(from*2)] = to>>8;
  writeBlock(startPartition+1+FATsector, buffer);
}

void reverse(char* str, int len)
{
  int i = 0, j = len - 1, temp;
  while (i < j) {
    temp = str[i];
    str[i] = str[j];
    str[j] = temp;
    i++;
    j--;
  }
}
int intToStr(uint16_t x, char str[], int d)
{
  int i = 0;
  while (x) {
    str[i++] = (x % 10) + '0';
    x = x / 10;
  }
  while (i < d)
    str[i++] = '0';
  reverse(str, i);
  str[i] = '\0';
  return i;
}
void ftoa(float n, char* res)
{
  if(n<0){res[0]='-';n*=-1;}
  else res[0] = '+';
  res++;
  int ipart = (int)n;
  float fpart = n - (float)ipart;
  int i = intToStr(ipart, res, 0);
  res[i] = '.';
  fpart = fpart * 100;
  intToStr((int)fpart, res + i + 1, 2);
  (res+i+1+2)[0] = 176;
  (res+i+1+2)[1] = 'C';
  (res+i+1+2)[2] = '\n';
}
