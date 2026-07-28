#!/bin/bash
# Run on VPS: bash /tmp/compare-local-to-server.sh
# Expects /tmp/local-md5.txt (hash path per line)

set -e
cd /root/chat_room_server

echo "=== DIFFER (content changed on server vs your PC) ==="
while read -r hash path; do
  [ -z "$path" ] && continue
  if [ ! -f "$path" ]; then
    echo "LOCAL_ONLY $path"
    continue
  fi
  rh=$(md5sum "$path" | awk '{print $1}')
  if [ "$rh" != "$hash" ]; then
    echo "DIFFER $path"
  fi
done < /tmp/local-md5.txt

echo ""
echo "=== LOCAL_ONLY (on PC, missing on server) ==="
while read -r hash path; do
  [ -z "$path" ] && continue
  [ ! -f "$path" ] && echo "LOCAL_ONLY $path"
done < /tmp/local-md5.txt

echo ""
echo "=== REMOTE_ONLY (on server, not in your PC manifest) ==="
for d in src include common; do
  [ -d "$d" ] || continue
  find "$d" -type f | sort | while read -r f; do
    grep -qF "  $f" /tmp/local-md5.txt || echo "REMOTE_ONLY $f"
  done
done

echo ""
echo "=== CmakeLists.txt ==="
if [ -f CmakeLists.txt ]; then
  rh=$(md5sum CmakeLists.txt | awk '{print $1}')
  lh=$(grep ' CmakeLists.txt$' /tmp/local-md5.txt | awk '{print $1}')
  if [ "$rh" = "$lh" ]; then
    echo "SAME CmakeLists.txt"
  else
    echo "DIFFER CmakeLists.txt"
  fi
else
  echo "LOCAL_ONLY CmakeLists.txt"
fi

echo ""
echo "=== SAME count (unchanged on server) ==="
same=0
while read -r hash path; do
  [ -z "$path" ] && continue
  [ -f "$path" ] || continue
  rh=$(md5sum "$path" | awk '{print $1}')
  [ "$rh" = "$hash" ] && same=$((same+1))
done < /tmp/local-md5.txt
echo "$same files match (not listed above)"
