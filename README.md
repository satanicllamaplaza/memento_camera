# memento_camera

## Button Lay outs.
- [ ] Declare an array of menu options (led brightness, led color, resolution, filters).
- [ ] Use the Left and Right buttons to index through that array of options.
- [ ] Have the selected array refernece a corresponding array of options.
- [ ] Use the Up and down Arrows to cycle through those corresponding arrays values.

### Needs investigation, can the camera support playback functionality on the display?
- [ ] Use the left diplay button to turn on and off playback.
- [ ] In Display mode use the Left and Right buttons to scroll through the photos.
- [ ] Use the Right displat button to perform a Delete or Select photo function.

## In depth filter building

Adafruit_PyCamera is a wrapper around Espressif’s esp32-camera library, which directly
talks to the OV2640/OV5640 sensor registers.
[Link to detailed options and modifications](https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/?utm_source=chatgpt.com)
I want to experiment with options at this level. Maybe set some defaults that are more
unique and stylized but also create some hardware level filters.
#### This will require me to properly manage my filters array in a way that supports both
#### native and custom filters.
Custom filters need to lean heavily on this lower level configuration. and post processing
will result in latancy and unusable diplay functionality.
