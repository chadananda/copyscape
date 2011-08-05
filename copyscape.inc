<?php


// $Id$

/**
 * @file
 * 
 */
  
define('COPYSCAPE_API_URL', 'http://www.copyscape.com/api/');

function copyscape_api_text_search($text, $encoding='ISO-8859-1', $full=NULL, $operation='csearch')  {
  $params['e']=$encoding; 
  if (isset($full))  $params['c']=$full; 
  return copyscape_api_call($operation, $params, array(2 => array('result' => 'array')), $text);
}
 
function copyscape_api_call($operation, $params=array(), $xmlspec=NULL, $postdata=NULL)  {
  $username = variable_get('copyscape_username', '');
  $apikey = variable_get('copyscape_apikey', '');
  if (!$username || !$apikey) return;
  // full api URL
  $url = COPYSCAPE_API_URL .'?u='. urlencode($username) .'&k='. urlencode($apikey) .'&o='. urlencode($operation);
  foreach ($params as $name => $value) $url .= '&'. urlencode($name) .'='. urlencode($value);
  // request data
  $response = copyscape_post_url($url, $postdata);
  if (strlen($response)) return copyscape_read_xml($response, $xmlspec); 
}
  
function copyscape_post_url($url, $postdata) {
  $curl=curl_init(); 
  curl_setopt($curl, CURLOPT_URL, $url);
  curl_setopt($curl, CURLOPT_TIMEOUT, 30);
  curl_setopt($curl, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($curl, CURLOPT_POST, isset($postdata)); 
  if (isset($postdata)) curl_setopt($curl, CURLOPT_POSTFIELDS, $postdata);
  $response=curl_exec($curl);
  curl_close($curl);
  return $response;
} 

function copyscape_read_xml($xml, $spec=NULL)  {
  global $_copyscape_xml_data, $_copyscape_xml_depth, $_copyscape_xml_ref, $_copyscape_xml_spec;
  $_copyscape_xml_data=array();
  $_copyscape_xml_depth=0;
  $_copyscape_xml_ref=array();
  $_copyscape_xml_spec=$spec;
  $parser=xml_parser_create();
  xml_set_element_handler($parser, 'copyscape_xml_start', 'copyscape_xml_end');
  xml_set_character_data_handler($parser, 'copyscape_xml_data');
  if (!xml_parse($parser, $xml, TRUE)) return FALSE;-
  xml_parser_free($parser);
  return $_copyscape_xml_data;
}

function copyscape_xml_start($parser, $name, $attribs)  {
  global $_copyscape_xml_data, $_copyscape_xml_depth, $_copyscape_xml_ref, $_copyscape_xml_spec;
  $_copyscape_xml_depth++;
  $name=strtolower($name);  
  if ($_copyscape_xml_depth==1)
    $_copyscape_xml_ref[$_copyscape_xml_depth]=&$_copyscape_xml_data;
  else {
    if (!is_array($_copyscape_xml_ref[$_copyscape_xml_depth-1]))
      $_copyscape_xml_ref[$_copyscape_xml_depth-1]=array();
    
    if (@$_copyscape_xml_spec[$_copyscape_xml_depth][$name]=='array') {
    if (!is_array(@$_copyscape_xml_ref[$_copyscape_xml_depth-1][$name])) {
      $_copyscape_xml_ref[$_copyscape_xml_depth-1][$name]=array();
      $key=0;
    } 
    else
      $key=1+max(array_keys($_copyscape_xml_ref[$_copyscape_xml_depth-1][$name]));
      $_copyscape_xml_ref[$_copyscape_xml_depth-1][$name][$key]='';
      $_copyscape_xml_ref[$_copyscape_xml_depth]=&$_copyscape_xml_ref[$_copyscape_xml_depth-1][$name][$key];
    } 
    else {
      $_copyscape_xml_ref[$_copyscape_xml_depth-1][$name]='';
      $_copyscape_xml_ref[$_copyscape_xml_depth]=&$_copyscape_xml_ref[$_copyscape_xml_depth-1][$name];
    }
  }
}

function copyscape_xml_end($parser, $name)  {
  global $_copyscape_xml_depth, $_copyscape_xml_ref;  
  unset($_copyscape_xml_ref[$_copyscape_xml_depth]);
  $_copyscape_xml_depth--;
}
function copyscape_xml_data($parser, $data)  {
  global $_copyscape_xml_depth, $_copyscape_xml_ref;
  if (is_string($_copyscape_xml_ref[$_copyscape_xml_depth])) $_copyscape_xml_ref[$_copyscape_xml_depth] .= $data;
}

