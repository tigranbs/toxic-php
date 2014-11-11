<?php

function Start($var)
{

    class Toxic
    {
        public static $output="";
    }
    
    function Call_Output($str)
    {
        Toxic::$output .= $str;
    }    
    echo 'aaaaaaaaa';
    return Toxic::$output;
}