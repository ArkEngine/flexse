rm -f data/index/day/0/index.*
rm -f data/index/day/1/index.*
rm -f data/index/day/2/index.*
rm -f data/index/his/0/index.*
rm -f data/index/his/1/index.*

echo file_no  : 0 > ./data/index/check_point
echo block_id : 0 >> ./data/index/check_point
rm -f data/idmap/idmap.*
rm -f data/attr/*_bitmap
rm -f data/attr/*_bitlist
./bin/flexse
