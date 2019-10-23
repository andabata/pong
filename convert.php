<?php
$z=0;
$max=0;

$in=fopen("bitmaps.txt","r");
while(!feof($in)) {
  $line=trim(fgets($in,100));
  //print_r($line);
  $cmd=substr($line,0,2);
  $cnt=substr($line,2);
  if($cmd=="--"){
    $name=$cnt;
  }
  if(is_numeric($cmd) and $cmd!="00"){
    $data[$cmd]=$cnt;
  }

  if($cmd=="##"){
    echo "//$name \n {";
    for($i=0;$i<32;$i++){
      $d0=($data["01"][$i]=='.'?0:1<<3);
      $d1=($data["02"][$i]=='.'?0:1<<2);
      $d2=($data["03"][$i]=='.'?0:1<<1);
      $d3=$data["04"][$i]=='.'?0:1;
      $D=$d0+$d1+$d2+$d3;
      print("$D,");
      $d0=($data["05"][$i]=='.'?0:1<<3);
      $d1=($data["06"][$i]=='.'?0:1<<2);
      $d2=($data["07"][$i]=='.'?0:1<<1);
      $d3=$data["08"][$i]=='.'?0:1;
      $D=$d0+$d1+$d2+$d3;
      print("$D,");
    }
    for($i=0;$i<32;$i++){
      $d0=($data["09"][$i]=='.'?0:1<<3);
      $d1=($data["10"][$i]=='.'?0:1<<2);
      $d2=($data["11"][$i]=='.'?0:1<<1);
      $d3=$data["12"][$i]=='.'?0:1;
      $D=$d0+$d1+$d2+$d3;
      print("$D,");
      $d0=($data["13"][$i]=='.'?0:1<<3);
      $d1=($data["14"][$i]=='.'?0:1<<2);
      $d2=($data["15"][$i]=='.'?0:1<<1);
      $d3=$data["16"][$i]=='.'?0:1;
      $D=$d0+$d1+$d2+$d3;
      print("$D,");
    }
    echo "}\n";
  }

}
