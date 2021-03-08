# ![Icon](icon.bmp "Icon")DSi Language Patcher

The DSi Language patcher is a small tool, which runs on your DSi (homebrew execution required) and create a copy of your original app launcher on the device root as "Launcher.dsi" and applies some patches to it, so it allows the use of other languages than available in your region.

### Related work

The language patcher stands on the shoulders of people, who worked on homebrew DSi packages and tools for more than a decade.

The Language Patcher uses work from

* https://github.com/WinterMute/twltool for decrypting the modcrypt sections
*  https://github.com/DS-Homebrew/GodMode9i for the nand-io driver working with libfat
* https://github.com/devkitPro/libnds for libnds
* https://github.com/devkitPro/libfat for accessing the file system
* https://github.com/devkitPro for the general framework/tooling

You can find the source and the corresponding licenses on the linked pages.

## License(s)
Please refer to the above mentioned resources to find the license, authors and sources of the parts not originated by me.
I am aware, that it is difficult for some of the code to be attributed correctly, as the immediate source might not be the root source. I will try to correct that and separate such code within own source files and attribution.

If anyone finds his original work and wants to be attributed asap. Please open an issue and I will give it priority.

My own code within this shall be distributed under [GPLv3](https://www.gnu.org/licenses/gpl-3.0)

## Copyright
The language patcher works over/on copyrighted work of a 3rd party. Distribution of the copyrighted or it's derived binary (Patched Launcher) is likely illegal in your country. Do not do this!

## Disclaimer
All operations modifying the internal NAND on the DSi inherit a probability to cause a by software non-recoverable state of the hardware (Bricking) which requires direct access to the hardware and a valid backup for the given DSi.

The author(s) are not liable for any damage or costs caused by the use of this software or any derived work.

Please always test a derived work in a suitable emulator (i.e. [no$cash Debugger](https://problemkaputt.de/gba-dev.htm))

## Distribution
This git only contains the source code. No binary (.nds/.dsi) file is provided but can be created from the source code with help from the above liked framework and libraries.





 

  
