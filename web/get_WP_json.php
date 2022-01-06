<?php
$myfile = fopen("/mnt/RAMDisk/zaehlerdata/stromzaehler.txt", "r") or die("Unable to open file!");
echo fgets($myfile);
fclose($myfile);
// echo "{\"WP_Pges\": 1705,\"WP_Wges\": 1234.5678}";
?>