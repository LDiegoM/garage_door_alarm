import jsmin
import glob

def processHtml(path):
    print("========================================")
    print("Processing: " + path)
    output = path.replace("../src/", "../data/")
    print("Output: " + output)
    code = open(path, "r").read()
    fullmin = jsmin.jsmin(code)
    dest = open(output, "w")
    dest.write(fullmin)
    dest.close()

for file in glob.glob("../src/wwwroot/*.html"):
    processHtml(file)

for file in glob.glob("../src/wwwroot/*/*.html"):
    processHtml(file)
