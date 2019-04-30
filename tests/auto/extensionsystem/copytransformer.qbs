import qbs
import qbs.File
import qbs.FileInfo

Transformer {
    property pathList sourceFiles
    property path targetDirectory
    inputs: sourceFiles
    Artifact { fileName: targetDirectory }
    prepare: {
        var commands = []
        for (var tag in inputs) {
            for (var index in inputs[tag]) {
                var artifact = inputs[tag][index];
                var cmd = new JavaScriptCommand();
                cmd.sourceFile = artifact.fileName;
                cmd.description = "Copying '" + cmd.sourceFile + "' to '" + output.fileName + "/'.";
                cmd.highlight = "codegen";
                cmd.targetFilePath = output.fileName + '/' + FileInfo.fileName(cmd.sourceFile);
                cmd.sourceCode = function() { File.copy(sourceFile, targetFilePath); }
                commands.push(cmd);
            }
        }
        return commands;
    }
}
