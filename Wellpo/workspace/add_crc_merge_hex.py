#!/usr/bin/env python
import os

def get_hex_file_type(inFileName):
    ret = "unknow"
    srcfile = open(inFileName, 'r')
    lines = srcfile.readlines()
    srcfile.close()
    
    print(inFileName)
    #print(srcData)
    for line in lines:
        if line[8] == '0':
            start_addr = line[3] + line[4] + line[5] + line[6] 
            if start_addr == "2000":
                ret = "app"
            elif start_addr == "0000":
                ret = "bootloader"                          
                
    print(ret)
    return ret
                   
    
def merge_hex_file(bootloader,app):
    dest_data = []
    print("merge_hex_file")
    with open(bootloader,'r') as boot_file:
        data_lines = boot_file.readlines()        
        dest_data.extend(data_lines)
        boot_file.close()

        for data_line in data_lines:#1.remove the data line that file end line.            
            if data_line[8] == '1':#file end line.
                #print(data_line)
                dest_data.remove(data_line)
                
        
    with open(app,'r') as app_file:
        data_lines = app_file.readlines()
        dest_data.extend(data_lines)#2.add app.hex data to the end of the bootloader.hex data.
        app_file.close()
                
    
    #dest_data.append(":00000001FF")#add end-line.
    #dest_data.insert(0,":020000040000FA\n")#add the base address.
    #print(type(dest_data),len(dest_data))

    dest_name = app.replace(".hex","_ubc.hex")#3.creat a new file    
    print("creat file:",dest_name)
    
    with open(dest_name,'w') as dest_file:
        dest_file.write(''.join(dest_data))#4.write the data to this new file.
        dest_file.close()

def add_crc_2bin_file(inFileName):
    srcfile = open(inFileName, 'rb')
    srcData = srcfile.read()
    srcfile.close()
        
    #desfile.write(srcData)
    checksum1 = 0
    checksum2 = 0
    i = 0
    while i < len(srcData)-4:
        checksum1 += srcData[i]
        checksum2 += srcData[i]
        i += 1
    while i < len(srcData):
        checksum2 += srcData[i]
        i += 1
    
    checksum1_bytes = checksum1.to_bytes(4,byteorder='little')
    checksum2_bytes = checksum2.to_bytes(4,byteorder='little')
    
    if checksum1_bytes != srcData[-4:]:
        outFileName = os.path.splitext(inFileName)[0] + "crc.bin"
        desfile = open(outFileName, 'wb')
        desfile.write(srcData)
        desfile.write(checksum2_bytes)
        desfile.close()
        print("create crc file:" + str(outFileName))
    else:
        print("file include CRC (4 bytes)")

if __name__ == '__main__':
    bin_files = [];
    bootloader_files = [];
    app_files = [];
    print("Start")
    file_names = os.listdir()
    print(file_names)
    for file_name in file_names:
        if file_name.endswith(".bin") and not file_name.endswith("crc.bin"):
            bin_files.append(file_name)
            
        if file_name.endswith(".hex") and not file_name.endswith("ubc.hex"):  
            file_type = get_hex_file_type(file_name)
            if file_type == "bootloader":
                bootloader_files.append(file_name)

            elif file_type == "app":  
                app_files.append(file_name)

    for bin_file in bin_files:
        add_crc_2bin_file(bin_file)
        
    if len(bootloader_files) == 1:  
        bootloader_file = bootloader_files[0]
        for app_file in app_files:
            merge_hex_file(bootloader_file,app_file)
        
           
             

        
