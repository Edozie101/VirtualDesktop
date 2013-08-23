cd "c:\Program Files (x86)\VideoLAN\VLC"

echo EXPORTS > libvlc.def
for /f "usebackq tokens=4,* delims=_ " %i in (`dumpbin /exports "c:\Program Files (x86)\VideoLan\VLC\libvlc.dll"`) do if %i==libvlc echo %i_%j >> libvlc.def

lib /def:"C:\Program Files (x86)\VideoLAN\VLC\libvlc.def" /out:"C:\Program Files (x86)\VideoLAN\VLC\libvlc.lib" /machine:x86




cd "c:\Program Files\VideoLAN\VLC"

echo EXPORTS > libvlc.def
for /f "usebackq tokens=4,* delims=_ " %i in (`dumpbin /exports "c:\Program Files\VideoLan\VLC\libvlc.dll"`) do if %i==libvlc echo %i_%j >> libvlc.def

lib /def:"C:\Program Files\VideoLAN\VLC\libvlc.def" /out:"C:\Program Files\VideoLAN\VLC\libvlc.lib" /machine:x86
