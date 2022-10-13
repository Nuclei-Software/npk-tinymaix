import os

PROGRAM_UNKNOWN="unknown"
def find_index(key, arr):
    try:
        index = arr.index(key)
    except:
        index = -1
    return index

def parse_benchmark(lines, lgf=""):
    if isinstance(lines, list) == False:
        return PROGRAM_UNKNOWN, PROGRAM_UNKNOWN, None
    subtype = "unknown"
    program_type = "tinymaix"
    lgf = lgf.replace("\\", "/")
    appnormdirs = os.path.dirname(os.path.normpath(lgf)).replace('\\', '/').split('/')

    index = find_index("examples", appnormdirs)
    if index >= 0:
        subtype = appnormdirs[index + 1]
    program_type = "%s_%s" % (program_type, subtype)
    result = dict()
    for line in lines:
        stripline = line.strip()
        if stripline.startswith("===tm_run use"):
            value, unit = stripline.split()[-2:]
            key = "time/%s" % (unit)
            result[key] = value
        if stripline.startswith("Total param"):
            try:
                # example line
                # Total param ~88.4 KB, OPS ~3.08 MOPS, buffer 11.0 KB
                paramlist = [ item.strip().split() for item in stripline.split(',') ]
                for valuelist in paramlist:
                    value = valuelist[-2].strip("~")
                    key = valuelist[-3] + '/' + valuelist[-1]
                    # param/KB: 88.4
                    result[key] = value
            except:
                continue

    return program_type, subtype, result
