/**
 * @file qt_modbus_client.h
 * @author Kate Colwell <kcolwell@irisdynamics.com>
 *
 * @brief  Virtual device driver for Modbus client serial communication using qt ports and timers
 *
 * This class extends the virtual ModbusClient base class
 *
 * @version 2.2.0
    
    @copyright Copyright 2022 Iris Dynamics Ltd 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    For questions or feedback on this file, please email <support@irisdynamics.com>. 
 */

#pragma once 

#include <QCoreApplication>
#include <QtSerialPort/QtSerialPort>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QIODevice>
#include "modbus_client.h"
#include "transaction.h"

 /**
  * @class windows_ModbusClient
  * @brief Extension of the ModbusClient virtual class that implements functions for the Eagle K20's timers, and interrupts. UART channel specific functions to be
 */

class qt_ModbusClient : public QObject, public ModbusClient {

    QT_USE_NAMESPACE
        Q_OBJECT

protected:

    int channel_number;
    uint32_t cycles_per_us;

public:

    //qt stuff
    QElapsedTimer systemTimer;
    QString my_port_name;
    QSerialPort* Port;
    QByteArray sendBuf;

    qt_ModbusClient(int _channel_number, uint32_t _cycles_per_us) : QObject(), ModbusClient(_channel_number, _cycles_per_us)
    {
        channel_number = _channel_number;
        cycles_per_us = _cycles_per_us;
        my_port_name = "COM" + QString::number(_channel_number);

        //check available ports for the requested port
        const auto infos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo& info : infos) {
            if (info.portName() == my_port_name) {
                Port = new QSerialPort(info);
                init(UART_BAUD_RATE);
                Port->open(QIODevice::ReadWrite);
            }
        };

        //set up bytes recieved signal/slot
        QObject::connect(Port, &QSerialPort::readyRead, this, &qt_ModbusClient::uart_isr);

        //start a qt elapsed timer 
        systemTimer.start();

        reset_state();
        QString currStateString = my_state;
    }

    /**
     * @brief Intializes the devices timers and sets initial interframe delay timer
     * @param baud The UART baud rate as defined in the client_config.h file
    */
    void init(int baud) override {
        Port->setBaudRate(baud);
        Port->setDataBits(QSerialPort::Data8);
        Port->setStopBits(QSerialPort::OneStop);
        Port->setParity(QSerialPort::EvenParity);
        Port->setFlowControl(QSerialPort::NoFlowControl); 
        reset_state();
    }

    //////////// Virtual function implementations ////////////

    /**
     * @brief Get the number of microseconds elapsed since the start of the timer/application
     * @return the elapsed time in microseconds
    */
    uint64_t get_system_time_us() {
        return systemTimer.nsecsElapsed() / 1000;
    }

public:

    /**
     * @brief If there are bytes left to send, add them to a buffer, then send them all together.
    */
    void tx_enable() override {
        while (messages.get_active_transaction()->bytes_left_to_send()) {
            if (my_state == emission) {
                send();
            }
        }
        Port->write(sendBuf);
        Port->flush();
        sendBuf.chop(sendBuf.size());
    }

    /**
     * @brief Not using interupts, so no implementation needed.
    */
    void tx_disable() override {

    }

    /**
     * @brief Load the send buffer with the next byte
     * @param byte		The byte to be transmitted.
     */
    void send_byte(uint8_t data) override {
        sendBuf.append(data);
    }

    /**
     * @brief Return the next byte received by the serial port.
     */
    uint8_t receive_byte() override {
        char byte = 0;
        char* data = &byte;
        if (Port->getChar(data)) {
            return (uint8_t)byte;
        }
        else {
            return 0;
        }
    }

    /**
     * @brief Adjust the baud rate
     * @param baud_rate the new baud rate in bps
     * this method overrides the modbus default delay
    */
    void adjust_baud_rate(uint32_t baud_rate_bps) override {
        Port->setBaudRate(baud_rate_bps);
    }

    /**
    * @brief Get the device's current system time in cycles - not using cycles, so just returns elapsed time in microseconds
    */
    uint32_t get_system_cycles() override {
        return systemTimer.nsecsElapsed() / 1000;
    };

    /**
   * @brief Slot called whenever there is new data to recieve in the serial port.
   */
    void uart_isr() override {
        if (my_state == reception) {
            while (Port->bytesAvailable()) {
                receive();
                //slot called when new data arrives in the port 
                //as long as the state is reception and there is data to recieve, it will be recieved. 
                //receive method handles sloting data into the correct transaction.
            }
        }
    }

    /**
    * @brief checks port to determine if at least one byte is ready to be read
    */
    bool byte_ready_to_receive()override {
        return (Port->bytesAvailable() > 0);
    };

    ~qt_ModbusClient() {
        Port->close();
    }
};

extern qt_ModbusClient modbus_client;
