# Voltages to Keyframes

**TODO: Frequency input to waveform visualization so that it does not flicker**

## Usage Notes
The dev setup assumes you are on linux (I am on Ubuntu 23, but not sure that changes any file paths)

Change your RACK_USER_DIR environment variable to the location of you .Rack2 folder if you want to use the "dev_build_and_install" script.
The script installs the plugin and moves it to the directory where it can be seen by rack when it is in development mode. Development mode allows you to see the output of stuff that you print to the log in the standard output of the terminal, very useful.

also, for saving the keyframes, set your VCV_DEV environment variable to the folder that contains this repo.

Before you use the ```dev_build_and_install``` script, run VCV rack in dev mode once. It will construct a new "plugins" folder in the Rack user folder (the .Rack2 folder) which will be needed by the script.

To run Rack in dev mode, use the command:

```
Rack --dev
```
OR
```
Rack -d
```

You can also use the ```-u``` flag to manually set the location of your user directory if you'd like to have a separate one just for development

It looks like the only way to save text files is if you run VCV in develop mode (probably for good reason). Not sure if there is a workaround, but for now do it this way.
