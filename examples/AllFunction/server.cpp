/**
 * @file      server.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-16
 *
 */
#include <WebServer.h>
#include <esp_camera.h>

WebServer server(80);

void handle_jpg_stream(void)
{
    WiFiClient client = server.client();
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    server.sendContent(response);

    while (client.connected())
    {
        // Serial.printf("%s :[%u] %u\n", __func__, millis(), esp_get_free_heap_size());
        yield();

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("fb empty");
            continue;
        }
        response = "--frame\r\n";
        response += "Content-Type: image/jpeg\r\n\r\n";
        server.sendContent(response);
        
        client.write(fb->buf, fb->len);
        server.sendContent("\r\n");
        esp_camera_fb_return(fb);
    }
    Serial.println("client disconnected!");
}

void handleNotFound()
{
    String message = "Server is running!\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    server.send(200, "text/plain", message);
}

void setupServer()
{
    server.on("/", HTTP_GET, handle_jpg_stream);
    server.onNotFound(handleNotFound);
    server.begin();
}

void loopServer()
{
    server.handleClient();
}
