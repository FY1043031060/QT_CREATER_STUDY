import qbs

Project {
    name: "Tools"
    references: [
        "qtcdebugger/qtcdebugger.qbs",
        "qtcreatorcrashhandler/qtcreatorcrashhandler.qbs",
        "qtpromaker/qtpromaker.qbs",
        "sdktool/sdktool.qbs",
        "valgrindfake/valgrindfake.qbs",
        "3rdparty/iossim/iossim.qbs",
        "iostool/iostool.qbs"
    ].concat(project.additionalTools)
}
