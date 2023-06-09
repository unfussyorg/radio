# radio

Built on the [Baldram ESP32 Library](https://github.com/baldram/ESP_VS1053_Library) for use with ESP32 & VS1053 boards. This is an internet radio meant to be installed before the amplifier circuit in a KLH Twenty One radio, with no irreversible mods.

#### Notes

-   Tried using the original KLH variable plate capacitors for tuning-- by reading the values on a separate Nano chip. Failed, couldn't access solder points for the capacitors without destroying the original part.
-   Instead, tuning is controlled with a 10k potentiometer. Using map() to make the stations line up with FM markings on the KLH dial. Odd values in the map are stations, even are dead zones. Rotary encoder could do without dead zones, but pot readings are too volatile. I also average each pot reading over 10 loops before using it.
-   Ring buffer for streaming was taken from the Baldram Library, helps a lot with streaming stability.
-   Some streams work better with HTTP 1.0. Added a property for each station to switch from 1.1 to 1.0. If sound is choppy with 1.1, try 1.0.
-   I was having trouble trying to get my potentiometer to map accurately to the FM dial. Then I realized the problem: an FM scale isn't linear. I could have used the exact pot value for each dial marking, but it's too much work. I set the map() so that stations line up to the first and last dial marking (88 & 110FM), and in between they vary from a little early to a little late.
-   This is not optimized in any way, I'm not a developer. It works for me.

#### TBD

There is no real-time sensory feedback while using the tuning pot, which is a usability issue. Station loading is not fast enough to be considered interactive. The only way to predictably change the station is to watch the dial markings while turning the pot. An analog tuner gives constant feedback— the signal continuously moves between static and clarity. Muscle memory has me reaching for the dial without looking, which makes this digital radio unsatisfying to use.

I'm planning to add a coin-sized vibration motor to the back of the pot. Hoping to get something that feels like a divot in the dial when you land on a station. This feedback is (hopefully) fast enough to allow no-look tuning of the radio. The strength is supposedly tunable by varying supply current.

Triggering a static mp3 is another option, more closely mirrors an analog radio experience. But static is annoying, and haptic feedback will work at any radio volume.

<br>
<br>

<img src="https://github.com/unfussyorg/radio/assets/121210659/003af917-9dcb-4ddd-be31-c31f8e9c14f3" height="400">
<img src="https://github.com/unfussyorg/radio/assets/121210659/abbc4510-70ae-45c3-8341-2a3883c0109c" height="400">

#### Based on library/applications:

-   [maniacbug/VS1053](https://github.com/maniacbug/VS1053)
-   [Esp-radio](https://github.com/Edzelf/Esp-radio)
-   [smart-pod](https://github.com/MagicCube/smart-pod)
-   [Baldram ESP32 Library](https://github.com/baldram/ESP_VS1053_Library)
