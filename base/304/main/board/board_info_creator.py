import sys
import os
import shutil
import io
import struct

def calc_crc(line):
    sum = 0
    for i in range(0, len(line), 2):
        byte = "0x"+line[i:i+2]
        #print(byte)
        sum += int(byte, 0)

    #print("sum:"+str(sum))
    sum = sum & 0xFF
    sum = ~sum
    #print("invert="+str(sum))
    sum += 1
    #print("add1="+str(sum))
    sum = sum & 0xFF
    #print("sum="+str(sum))
    sum_str = str(hex(sum))[2:].zfill(2).upper()
    #print(sum_str)
    return sum_str

def convert_char_to_hex_string(input):
    return str(hex(ord(input)))[2:].zfill(2).upper()

def swap32(x):
    return int.from_bytes(x.to_bytes(4, byteorder='little'), byteorder='big', signed=False)

def swap16(x):
    return int.from_bytes(x.to_bytes(2, byteorder='little'), byteorder='big', signed=False)

def main():
    print('Search board_info.txt file')
    file_name = "board_info.txt"
    cur_dir = os.getcwd()
    file_list = os.listdir(cur_dir)

    if file_name in file_list:
        print('Found board_info.txt')
        print('Read info file')
        settings_file = open(cur_dir+'\\'+file_name, "r")
        settings_string_lines = settings_file.readlines()

        for line in settings_string_lines:
            if "address=" in line:
                address = line[len("address="):-1]
            if "hardware_number=" in line:
                hardware_number = line[len("hardware_number="):-1]
            if "serial_number=" in line:
                serial_number = line[len("serial_number="):-1]
            if "hardware_name=" in line:
                hardware_name = line[len("hardware_name="):-1]
            if "hardware_version=" in line:
                hardware_version= line[len("hardware_version="):-1]
            if "hardware_revision=" in line:
                hardware_revision = line[len("hardware_revision="):-1]
            if "hardware_function=" in line:
                hardware_function = line[len("hardware_function="):-1]
            if "board_family_number=" in line:
                board_family_number = line[len("board_family_number="):-1]
            if "compatibility_number=" in line:
                compatibility_number = line[len("compatibility_number="):-1]
            if "reserved=" in line:
                reserved = line[len("reserved="):]

        #write address line
        address_line = ":02000004" + address[0:4]
        address_line += calc_crc(address_line[1:])
        print(address_line)

        #write hardware_number
        hardware_number_line = ":0A" + address[4:] + "00"
        for c in hardware_number:
            hardware_number_line += convert_char_to_hex_string(c)
        hardware_number_line += calc_crc(hardware_number_line[1:])
        print(hardware_number_line)

        new_address = int("0x"+address[4:], 0)

        #write serial number
        new_address += 0xA
        serial_number_line = ":0C" + str(hex(new_address))[2:].zfill(4).upper() + "00"
        for c in serial_number:
            serial_number_line += convert_char_to_hex_string(c)
        serial_number_line += calc_crc(serial_number_line[1:])
        print(serial_number_line)

        #write hardware name
        new_address += 0xC
        hardware_name_line = ":10" + str(hex(new_address))[2:].zfill(4).upper() + "00"
        for c in hardware_name:
            hardware_name_line += convert_char_to_hex_string(c)
        for x in range(len(hardware_name), 16):
            hardware_name_line += hex(32)[2:]
        hardware_name_line += calc_crc(hardware_name_line[1:])
        hardware_name_line = hardware_name_line.upper()
        print(hardware_name_line)

        #write hardware version
        new_address += 0x10
        hardware_version_line = ":0E" + str(hex(new_address))[2:].zfill(4).upper() + "00"
        hardware_version_line += str(hex(int(hardware_version))[2:]).zfill(2)
        hardware_version_line += str(hex(int(hardware_revision))[2:]).zfill(2)
        hardware_version_line += str(hex(swap16(int(hardware_function)))[2:]).zfill(4)
        hardware_version_line += str(hex(swap16(int(board_family_number)))[2:]).zfill(4)
        hardware_version_line += str(hex(swap16(int(compatibility_number)))[2:]).zfill(4)
        hardware_version_line += str(hex(swap32(int(reserved)))[2:]).zfill(8)
        # make it a 52 bytes struct(for alignment)
        hardware_version_line += "0000"
        hardware_version_line += calc_crc(hardware_version_line[1:])
        hardware_version_line = hardware_version_line.upper()
        print(hardware_version_line)

        end_line = ":00000001"
        end_line += calc_crc(end_line[1:])
        print(end_line)

        print("Write " + serial_number + ".hex")
        output_file = open(serial_number + ".hex", "w")

        output_file.write(address_line)
        output_file.write('\n')
        output_file.write(hardware_number_line)
        output_file.write('\n')
        output_file.write(serial_number_line)
        output_file.write('\n')
        output_file.write(hardware_name_line)
        output_file.write('\n')
        output_file.write(hardware_version_line)
        output_file.write('\n')
        output_file.write(end_line)
        output_file.write('\n')
		
        output_file.close()


    else:
        print('board_info.txt file not found')


if __name__ == "__main__":
    main()