import json
dd={}
dd["termlist"] = ["a[1]", "b[1]"]
filt = []
f_type = {}
f_type["METHOD"] = "EQUAL"
f_type["VALUE"] = 4
filt.append(f_type)
f_duration = {}
f_duration["METHOD"] = "ZONE"
f_duration["MIN"] = 4
f_duration["MAX"] = 8
filt.append(f_duration)
dd["filter"] = filt
dd["ranking"] = {}
dd["merger"] = "weight_merge"
limit = {}
limit["min"] = 0
limit["max"] = 15
dd["limit"] = limit
dd["groupby"] = "type"
dd["sortby"] = "id"
print json.dumps(dd)
