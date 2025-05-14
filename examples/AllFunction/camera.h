/**
 * @file      camera.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-16
 *
 */
#pragma once

bool setupCamera();
void nextFrameSize();
bool setupCameraTask(const QueueHandle_t frame_o);