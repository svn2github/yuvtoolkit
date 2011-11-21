#ifndef SETTINGS_H
#define SETTINGS_H

#define SETTINGS_GET_RENDERER()        value("main/renderer", "D3D").toString()
#define SETTINGS_SET_RENDERER(v)       setValue("main/renderer", v)

#define SETTINGS_GET_ZOOM()            value("main/zoom", 1).toInt()
#define SETTINGS_SET_ZOOM(v)           setValue("main/zoom", v)

#define SETTINGS_GET_DIST_MAP()        value("measure/distmap", QStringList()<<"PSNR").toStringList()
#define SETTINGS_SET_DIST_MAP(v)       setValue("measure/distmap", v)

#define SETTINGS_GET_FILE_PATH()       value("main/openfilespath", "").toString()
#define SETTINGS_SET_FILE_PATH(v)      setValue("main/openfilespath", v)

#define SETTINGS_GET_SCRIPT_PATH()     value("main/openscriptpath", "").toString()
#define SETTINGS_SET_SCRIPT_PATH(v)    setValue("main/openscriptpath", v)

#define SETTINGS_GET_LOGGING()         value("main/logging", false).toBool()
#define SETTINGS_SET_LOGGING(v)        setValue("main/logging", v)

#define SETTINGS_GET_AUTO_RESIZE()     value("main/autoresize", true).toBool()
#define SETTINGS_SET_AUTO_RESIZE(v)    setValue("main/autoresize", v)

#endif