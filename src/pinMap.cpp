/*
  Copyright (c) 2014-2015 Arduino LLC.  All right reserved.
  Copyright (c) 2016 Sandeep Mistry All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "pinMap.h"

//nrf-arduino
const uint32_t g_ADigitalPinMap[] = {
    //digital pins=======================
    11,  //D0                      (11)
    12,  //D1                      (12)
    13,  //D2                      (13)
    14,  //D3                      (14)
    15,  //D4                      (15)
    16,  //D5                      (16)
    17,  //D6                      (17)
    18,  //D7                      (18)
    19,  //D8                      (19)
    20,  //D9                      (20)
    22,  //D10                     (22)
    23,  //D11                     (23)
    24,  //D12                     (24)
    25,  //D13                     (25)

    //analog pins=======================
    3,   //A0                      (3)   
    4,   //A1                      (4)   
    28,  //A2                      (28)     
    29,  //A3                      (29)      
    30,  //A4                      (30)       
    31,  //A5                      (31)          
    5,   //A6 AIN3 (P0.05)         (5)            
    2,   //A7 AIN0 (P0.02) / AREF  (2)             

    //i2c pins===========================
    26,  // SDA                    (26)       
    27,  // SCL                    (27)   

    //uart pins==========================
    8,  // RX                      (8)
    6   // TX                      (6)   
};
