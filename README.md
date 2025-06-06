# Codec Library
`Codec` Library help you to create your own layer protocol 

## Features
- Support multiple codec
- Support Serialize and Deserialize data
- Support customize codec configuration based on hardware
- HAL (Hardware Abstract Layer) support
- Support Encode/Decode in Async mode
- Support Encode/Decode a single frame
- Support Automatic remove padding or noise bytes between frames
- Support Decode Sync function for faster remove padding or noise bytes between frames

## Dependencies
- [Stream Library](https://github.com/Ali-Mirghasemi/Stream)

## Examples
- [Simple](./Examples/Simple/) shows basic usage of `Codec` Library
- [BasicFrame](./Examples/BasicFrame/) shows basic usage of `BasicFrame` Library
- [Codec-Test](./Examples/Codec-Test/) shows basic usage of `Codec` Library and test library
- [STM32F429-DISCO](./Examples/STM32F429-DISCO/) shows basic usage of `Codec` Library and how to port on STM32F429-DISCO
