<?php
$z=0;
$max=0;

$in=fopen("bitmaps.txt","r");
while(!feof($in)) {
  $line=fgets($in,100);
  print_r($line);


  list($NULL,$data0)=explode(" ",trim(fgets($in,100)));
  list($NULL,$data1)=explode(" ",trim(fgets($in,100)));
  list($NULL,$data2)=explode(" ",trim(fgets($in,100)));
  list($NULL,$data3)=explode(" ",trim(fgets($in,100)));
  list($NULL,$data4)=explode(" ",trim(fgets($in,100)));
  list($NULL,$data5)=explode(" ",trim(fgets($in,100)));
  list($NULL,$data6)=explode(" ",trim(fgets($in,100)));
  list($NULL,$data7)=explode(" ",trim(fgets($in,100)));

  $width[$z]=strlen($data0);
  if(strlen($data0)>$max) $max=strlen($data0);
  for($i=0;$i<strlen($data0);$i++){
    $d0=($data0[$i]=='.'?0:1<<3);
    $d1=($data1[$i]=='.'?0:1<<2);
    $d2=($data2[$i]=='.'?0:1<<1);
    $d3=$data3[$i]=='.'?0:1;
    $D=$d0+$d1+$d2+$d3;
    $d0=($data4[$i]=='.'?0:1<<3);
    $d1=($data5[$i]=='.'?0:1<<2);
    $d2=($data6[$i]=='.'?0:1<<1);
    $d3=$data7[$i]=='.'?0:1;
    $L=$d0+$d1+$d2+$d3;
    $upper[$z][]=$D;
    $lower[$z][]=$L;
  }
  $z++;
}

echo "char f_width[$z]={";
for($i=0;$i<$z;$i++) echo $width[$i].",";
echo "};\n";
echo "char f_upper[$z][$max]={";
for($i=0;$i<$z;$i++){
   echo "{";
   for($j=0;$j<$max;$j++) {
     if(isset($upper[$i][$j]))
      echo $upper[$i][$j].",";
     else
     echo "0,";
   }
   echo "},";
}
echo "};\n";
echo "char f_lower[$z][$max]={";
for($i=0;$i<$z;$i++){
   echo "{";
   for($j=0;$j<$max;$j++) {
     if(isset($lower[$i][$j]))
      echo $lower[$i][$j].",";
     else
     echo "0,";
   }
   echo "},";
}
echo "};\n";
?>


