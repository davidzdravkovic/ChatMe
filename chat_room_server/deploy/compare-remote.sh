#!/bin/bash
cd /root/chat_room_server || exit 1
echo "=== DIFFER (PC != server) ==="
while read -r hash path; do
  path=$(printf '%s' "$path" | tr -d '\r')
  [ -z "$path" ] && continue
  server_path="$path"
  if [ "$path" = "CmakeLists.txt" ] && [ ! -f "$path" ] && [ -f CMakeLists.txt ]; then
    server_path=CMakeLists.txt
  fi
  if [ ! -f "$server_path" ]; then
    echo "LOCAL_ONLY $path"
    continue
  fi
  rh=$(md5sum "$server_path" | awk '{print $1}')
  if [ "$rh" != "$hash" ]; then
    echo "DIFFER $path"
  fi
done < /tmp/local-md5.txt
echo ""
echo "=== LOCAL_ONLY (missing on server) ==="
while read -r hash path; do
  path=$(printf '%s' "$path" | tr -d '\r')
  [ -z "$path" ] && continue
  server_path="$path"
  if [ "$path" = "CmakeLists.txt" ] && [ -f CMakeLists.txt ]; then continue; fi
  [ ! -f "$server_path" ] && echo "LOCAL_ONLY $path"
done < /tmp/local-md5.txt
echo ""
diff=0; same=0
while read -r hash path; do
  path=$(printf '%s' "$path" | tr -d '\r')
  [ -z "$path" ] && continue
  server_path="$path"
  if [ "$path" = "CmakeLists.txt" ] && [ -f CMakeLists.txt ]; then server_path=CMakeLists.txt; fi
  [ ! -f "$server_path" ] && continue
  rh=$(md5sum "$server_path" | awk '{print $1}')
  if [ "$rh" = "$hash" ]; then same=$((same+1)); else diff=$((diff+1)); fi
done < /tmp/local-md5.txt
echo "Summary: $diff differ, $same identical (of $(wc -l < /tmp/local-md5.txt) tracked files)"
