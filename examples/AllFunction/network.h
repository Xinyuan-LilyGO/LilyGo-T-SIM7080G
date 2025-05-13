/**
 * @file      network.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-16
 *
 */
#pragma once

void setupNetwork(bool setup_AP_Mode);
void loopNetwork();
String getIpAddress();