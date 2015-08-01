#!/bin/sh
echo 268435456 > /sys/block/zram0/disksize
/system/bin/tiny_mkswap /dev/block/zram0
/system/bin/tiny_swapon /dev/block/zram0

echo 100 > /sys/kernel/mm/ksm/pages_to_scan
echo 500 > /sys/kernel/mm/ksm/sleep_millisecs
echo 1 > /sys/kernel/mm/ksm/run
echo 1 > /sys/kernel/mm/ksm/deferred_timer

