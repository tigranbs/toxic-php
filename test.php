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


	var_dump($_POST);

	echo aa();
}

