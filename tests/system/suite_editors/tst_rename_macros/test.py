#############################################################################
##
## Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
## Contact: http://www.qt-project.org/legal
##
## This file is part of Qt Creator.
##
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and Digia.  For licensing terms and
## conditions see http://qt.digia.com/licensing.  For further information
## use the contact form at http://qt.digia.com/contact-us.
##
## GNU Lesser General Public License Usage
## Alternatively, this file may be used under the terms of the GNU Lesser
## General Public License version 2.1 as published by the Free Software
## Foundation and appearing in the file LICENSE.LGPL included in the
## packaging of this file.  Please review the following information to
## ensure the GNU Lesser General Public License version 2.1 requirements
## will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
##
## In addition, as a special exception, Digia gives you certain additional
## rights.  These rights are described in the Digia Qt LGPL Exception
## version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
##
#############################################################################

source("../../shared/qtcreator.py")

cppEditorStr = ":Qt Creator_CppEditor::Internal::CPPEditorWidget"

def main():
    global cppEditorStr
    folder = prepareTemplate(os.path.abspath(os.path.join(os.getcwd(), "..", "shared",
                                                          "simplePlainCPP")))
    if folder == None:
        test.fatal("Could not prepare test files - leaving test")
        return
    proFile = os.path.join(folder, "testfiles.pro")
    startApplication("qtcreator" + SettingsPath)
    if not startedWithoutPluginError():
        return
    openQmakeProject(proFile)
    if not testRenameMacroAfterSourceModification():
        return
    addCPlusPlusFileToCurrentProject("anothertestfile.h", "C++ Header File")
    if not testRenameMacroAfterSourceMoving():
        return
    invokeMenuItem("File", "Save All")
    invokeMenuItem("File", "Exit")

def testRenameMacroAfterSourceModification():
    def __deleteAnyClass__():
        global cppEditorStr
        if platform.system() == 'Darwin':
            type(cppEditorStr, "<Meta+Left>")
        else:
            type(cppEditorStr, "<Home>")
        markText(cppEditorStr, "Down", 5)
        type(cppEditorStr, "<Delete>")

    test.log("Testing rename macro after modifying source.")
    formerTexts = {}
    content = openDocumentPlaceCursor("testfiles.Headers.testfile\\.h",
                                      "class AnyClass", __deleteAnyClass__)
    if not content:
        return False
    formerTexts["testfiles.Headers.testfile\\.h"] = content
    content = openDocumentPlaceCursor("testfiles.Sources.testfile\\.cpp", "SOME_MACRO_NAME(a)")
    if not content:
        return False
    formerTexts["testfiles.Sources.testfile\\.cpp"] = content
    performMacroRenaming('SOME_OTHER_MACRO_NAME')
    verifyChangedContent(formerTexts, "SOME_MACRO_NAME", "SOME_OTHER_MACRO_NAME")
    revertChanges(formerTexts)
    return True

def testRenameMacroAfterSourceMoving():
    def __cut__():
        global cppEditorStr
        if platform.system() == 'Darwin':
            type(cppEditorStr, "<Meta+Left>")
        else:
            type(cppEditorStr, "<Home>")
        markText(cppEditorStr, "Down", 4)
        invokeMenuItem("Edit", "Cut")

    def __paste__():
        global cppEditorStr
        type(cppEditorStr, "<Return>")
        invokeMenuItem("Edit", "Paste")

    def __insertInclude__():
        global cppEditorStr
        typeLines(cppEditorStr, ['', '#include "anothertestfile.h"'])

    test.log("Testing rename macro after moving source.")
    formerTexts = {}
    content = openDocumentPlaceCursor("testfiles.Headers.testfile\\.h",
                                      "#define SOME_MACRO_NAME( X )\\", __cut__)
    if not content:
        return False
    formerTexts["testfiles.Headers.testfile\\.h"] = content
    content = openDocumentPlaceCursor("testfiles.Headers.anothertestfile\\.h",
                                      "#define ANOTHERTESTFILE_H", __paste__)
    if not content:
        return False
    formerTexts["testfiles.Headers.anothertestfile\\.h"] = content
    content = openDocumentPlaceCursor('testfiles.Sources.testfile\\.cpp',
                                      '#include "testfile.h"', __insertInclude__)
    if not content:
        return False
    formerTexts["testfiles.Sources.testfile\\.cpp"] = content
    placeCursorToLine(cppEditorStr, "SOME_MACRO_NAME(a)")
    performMacroRenaming("COMPLETELY_DIFFERENT_MACRO_NAME")
    verifyChangedContent(formerTexts, "SOME_MACRO_NAME", "COMPLETELY_DIFFERENT_MACRO_NAME")
    revertChanges(formerTexts)
    return True

def performMacroRenaming(newMacroName):
    for i in range(10):
        type(cppEditorStr, "<Left>")
    invokeContextMenuItem(waitForObject(cppEditorStr), "Refactor",
                          "Rename Symbol Under Cursor")
    validateSearchResult(2)
    replaceLineEdit = waitForObject("{leftWidget={text='Replace with:' type='QLabel' "
                                    "unnamed='1' visible='1'} "
                                    "type='Find::Internal::WideEnoughLineEdit' unnamed='1' "
                                    "visible='1' "
                                    "window=':Qt Creator_Core::Internal::MainWindow'}")
    replaceEditorContent(replaceLineEdit, newMacroName)
    clickButton(waitForObject("{text='Replace' type='QToolButton' unnamed='1' visible='1' "
                              "window=':Qt Creator_Core::Internal::MainWindow'}"))

def verifyChangedContent(origTexts, replacedSymbol, replacement):
    global cppEditorStr
    successfullyCompared = []
    for fileName,text in origTexts.iteritems():
        if openDocument(fileName):
            successfullyCompared.append(test.compare(waitForObject(cppEditorStr).plainText,
                                                     text.replace(replacedSymbol, replacement),
                                                     "Verifying content of %s" %
                                                     simpleFileName(fileName)))
        else:
            successfullyCompared.append(False)
            test.fail("Failed to open document %s" % simpleFileName(fileName))
    if successfullyCompared.count(True) == len(origTexts):
        test.passes("Successfully compared %d changed files" % len(origTexts))
    else:
        test.fail("Verified %d files - %d have been successfully changed and %d failed to "
                  "change correctly." % (len(origTexts), successfullyCompared.count(True),
                                         successfullyCompared.count(False)))

def revertChanges(files):
    for f in files:
        simpleName = simpleFileName(f)
        if openDocument(f):
            try:
                invokeMenuItem('File', 'Revert "%s" to Saved' % simpleName)
                clickButton(waitForObject(":Revert to Saved.Proceed_QPushButton"))
                test.log("Reverted changes inside %s" % simpleName)
            except:
                test.warning("File '%s' cannot be reverted." % simpleName,
                             "Maybe it has not been changed at all.")
        else:
            test.fail("Could not open %s for reverting changes" % simpleName)
