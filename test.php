<?php 

function aa()
{
	echo "sdfsdfsd";
	return "string";
}


class Toxic
{
	public static $output_index;
}

function Start($url, $toxic_index)
{
	Toxic::$output_index = $toxic_index;

file_put_contents("/home/tigran/fffff", "Start", FILE_APPEND);
   
   
    file_put_contents("/home/tigran/fffff", "Output Function", FILE_APPEND);


echo aa();

file_put_contents("/home/tigran/fffff", "Output !!!", FILE_APPEND);
}

