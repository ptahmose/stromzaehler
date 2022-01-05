from machine import UART, Pin 
import time

# uos provides information such as the machine name and build version numbers
import uos

BT_Id = 0
BT_KEY = Pin(2, Pin.OUT)

# while True:
#     BT_KEY.on()
#     print("ON")
#     sleep(1)
#     BT_KEY.off()
#     print("OFF")
#     sleep(1)

def ConfigureBlueTooth():
    # we set the "KEY"-pin (of the BT-module) to high, in order to get into "AT-mode"
    uart_bt = UART(BT_Id,baudrate=38400,timeout = 1000)
    sleep(2)
    #uart_bt.init(baudrate=38400)
    uart_bt.write("AT+NAME=STROMZAEHLER\r\n")
    response = bytes()
    response = uart_bt.readline()
    print("SET NAME="+str(response))
    uart_bt.write("AT+NAME?\r\n")
    response = uart_bt.readline()
    print("GET NAME="+str(response))
    uart_bt.write("AT+BAUD6\r\n")  # Set to 38400 bauds
    response = uart_bt.readline()
    print("SET BAUD="+str(response))
    #uart_bt.deinit()
    


# # setup the UART
# id = 0
# rx = Pin(1)
# tx = Pin(0)
# baudrate=38400 # default is 9600
# 
# # create the UART
# #uart = UART(id=id, baudrate=baudrate,tx=tx, rx=rx, bits=8, parity=None, stop=1)
# uart = UART(0,38400)
# 
# def help():
#     print("AT commands")
#     print("-"*50)
#     print("AT               - Attention, should return 'OK'")
#     print("AT+ORGL          - restore default settings")
#     print("AT+VERSION       - returns the verison number")
#     print("AT+UART?         ")
#     print("AT+PSWD?         ")
#     print("AT+NAME?         - print out the current bluetooth device name")
#     print("AT+NAME<name>    - set the name of the device, e.g. AT+NAMERobot will set it to 'Robot'")
#     print("AT+PIN<pin>      - will set the pin e.g. 'AT+PIN4321' will set the pin to '4321'")
# 
# 
# 
# # Print a pretty command line
# print("-"*50)
# print("PicoTerm")
# print(uos.uname())
# print("type 'quit' to exit, or help for commands")
# # Loop
# command = ""
# while True and command !='quit':
#     # Write our command prompt
#     command = input("PicoTerm>")
#     if command == 'help':
#         help()
#     elif command != 'quit':
#         uart.write(command+"\r\n")
#         print(command)
#         sleep(0.1)
#         response = bytes()
#         
#         if uart.any() > 0:
#             response = uart.readline()
#             print("reading data")
#         #    print(response)
#         # output = "".join(["'",str(command),"'","response:",str(response.decode('utf-8'))])
#         try:
#             output = str(response.decode('utf-8'))
#         except:
#             print("weird response")
#         print(output)
#     elif command == 'quit':
#         print("-"*50)
#         print('Bye.')

#ConfigureBlueTooth()

uart_bt = UART(0,9600,tx=Pin(0),rx=Pin(1))
uart_ir = UART(1,baudrate=9600,tx=Pin(4),rx=Pin(5),timeout=0)

def ReadIRMessage():
    response = bytes()
    while True:
        if uart_ir.any()>0:
            data=uart_ir.read()
            response=b''.join([response,data])
        else:
            deadline = time.ticks_add(time.ticks_ms(),8)
            while time.ticks_diff(deadline,time.ticks_ms()) > 0:
                if uart_ir.any()>0:
                    break
                time.sleep_ms(1)
            if not (uart_ir.any()>0):
                return response
          
            

#uart_bt.init(baudrate=9600)
counter = 0

while True:
    #print("HELLO WORLD")
    #text = "TESTTEXT #"+str(counter)+"\r\n"
    BT_KEY.toggle()
    #uart_bt.write(text)
     #uart_ir.write(text)
#      while True:
#          if uart_ir.any()>0:
#              response = uart_ir.read(1)
#              print(" ".join(hex(n) for n in response))
#          else:
#              print("==============================")
#              break
    #response = uart_ir.read(2000);
    response = ReadIRMessage()
    #responseHex = "".join(hex(n).strip("0x") for n in response)
    responseHex = "".join(str.format('{:02X}',n) for n in response)
    uart_bt.write(responseHex+"\n")
    print(responseHex)
    #print(str(counter))
     #print(text)
    time.sleep(0.1)
    counter=counter+1
    
    
#    # print('checking BT')
#    if uart.any():
#        command = uart.readline()
#        print(command)

