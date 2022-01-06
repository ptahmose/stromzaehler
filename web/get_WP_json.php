<?php
$myfile = fopen("/mnt/RAMDisk/zaehlerdata/stromzaehler.txt", "r") or die("Unable to open file!");
if (flock($myfile,LOCK_SH))
{
    echo fgets($myfile);
    flock($myfile,LOCK_UN);
    fclose($myfile);
}
// echo "{\"WP_Pges\": 1705,\"WP_Wges\": 1234.5678}";
?>