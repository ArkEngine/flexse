import json
dd={}
dd["termlist"] = []
term = {}
term["term"] = "a[1]"
term["weight"] = 50
dd["termlist"].append(term)
term = {}
term["TERM"] = "b[1]"
term["WEIGHT"] = 50
dd["termlist"].append(term)
filt = []
f_type = {}
f_type["method"] = "EQUAL"
f_type["field"] = "type"
f_type["value"] = 4
filt.append(f_type)
f_duration = {}
f_duration["method"] = "ZONE"
f_duration["field"] = "duration"
f_duration["min"] = 4
f_duration["max"] = 8
filt.append(f_duration)
dd["filter"] = filt
dd["ranking"] = {}
dd["merger"] = "weight_merge"
limit = {}
limit["min"] = 0
limit["max"] = 15
dd["limit"] = limit
dd["groupby"] = "type"
#dd["sortby"] = "id"
print json.dumps(dd)
