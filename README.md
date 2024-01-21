# HamLab
## Introduction
HamLab aims to be a one stop applicication for controlling your shack.  From Logging for contesting and daily use, monitoring and controlling your radios, starting, stopping, monitoring and controling the myriad of other software out there, and maybe even configuring them.

It is a work in progress in the VERY early stages, so don't expect much if anything for now. Stay tuned.

## Contributing
For anyone wanting to help, it would be very much appreciated. HamLab is based on a plugin approach for just about everything. Plugins can either be dynamic libraries (.so, .dll) that are chosen, loaded and unloaded at run time, or static, which means they are always there. They can share data through a central data share, and can draw their own UI.

### Components
This is a list of the libraries used thus far:
- Dear ImGui: <https://github.com/ocornut/imgui.git>
- SDL2 (For windowing and events.): <https://libsdl.org>
- Boost Libraries: <https://www.boost.org>
- Hamlib (obviously): <https://github.com/Hamlib/Hamlib>
- json::document (Very fast and secure JSON (and XML) parser, writer and access library): <https://github.com/VA7ODR/json_document.git>
- shared_recursive_mutex (For Mutex hang logging. Also a WIP.): <https://github.com/VA7ODR/shared_recursive_mutex.git>
- TinyXML (For XML Parsing in future. Submodule of json::document for JSON/XML interchangability.): <https://github.com/VA7ODR/timyxml.git> (This version has a fix for CVE-2021-42260 that the original does not.) It is planned to remove TinyXML in future in preference for my own parser.
- ArbitraryOrderMap (Library that works like a std::map, but maintains the order given): <https://github.com/VA7ODR/ArbitraryOrderMap.git> (Primarily used by the ojson/odata::document classes which are subclasses of json/data::document.)
- SDString (Secure Delete String. std::basic_string derivative that zeros memory before freeing to thwart memory sniffers. Primarily used by json::document et al: <https://github.com/VA7ODR/SDString.git>
