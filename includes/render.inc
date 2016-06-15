<?php
/*
 * @file
 * Render functions.
 */

/**
 * Renders an element.
 *
 * This function renders an element using b_render().
 *
 * @param $element
 *   The element to be rendered.
 *
 */

/**
 * Assign $arguments and $options based on 
 * $command['arguments'] & $command[options]
 *
 */
function render_text($variables) {
  $text = $variables['value'];
  $foreground = array(
    'black' => '0;30',
    'dark_gray' => '1;30',
    'red' => '0;31',
    'bold_red' => '1;31',
    'green' => '0;32',
    'bold_green' => '1;32',
    'brown' => '0;33',
    'yellow' => '1;33',
    'blue' => '0;34',
    'bold_blue' => '1;34',
    'purple' => '0;35',
    'bold_purple' => '1;35',
    'cyan' => '0;36',
    'bold_cyan' => '1;36',
    'white' => '1;37',
    'bold_gray' => '0;37',
  );
  if(isset($variables['#color']) && isset($foreground[$variables['#color']])) {
    echo "\033[" . $foreground[$variables['#color']] . "m" . $text . "\033[0m";
  }
  else{
    echo $text;
  }
  echo "\n";
}

function table_color_text($variables){
  $text = $variables['data'];
  $foreground = array(
    'black' => '0;30',
    'dark_gray' => '1;30',
    'red' => '0;31',
    'bold_red' => '1;31',
    'green' => '0;32',
    'bold_green' => '1;32',
    'brown' => '0;33',
    'yellow' => '1;33',
    'blue' => '0;34',
    'bold_blue' => '1;34',
    'purple' => '0;35',
    'bold_purple' => '1;35',
    'cyan' => '0;36',
    'bold_cyan' => '1;36',
    'white' => '1;37',
    'bold_gray' => '0;37',
  );
  if(isset($variables['#color']) && isset($foreground[$variables['#color']])) {
    return "\033[" . $foreground[$variables['#color']] . "m" . $text . "\033[0m";
  }
  else{
    return $text;
  }
}
  
function b_render($elements){
//  print_r($elements);
  if(!is_array($elements)) {
    echo $elements."\n";
    return;
  }
  if(!isset($elements['#type'])){
    foreach($elements as $element){
      b_render($element);
    }
  }else{
    $element = $elements;
    $type = $element['#type'];
    $function = 'render_' . $type; 
    if(function_exists($function)) {
      $function($element);
    }
    if(isset($element['elements'])){
      b_render($element['elements']);
    }
  }
}

/**
 * Count total data width;
 */
function _render_table_row_col_length($rows){
  $cols_length = array();
  foreach($rows as $row) {
    $index = 0;
    foreach($row as $col) {
      if(isset($col['data'])){
        $length = mb_strlen($col['data']);
      }
      else{
        $length= mb_strlen($col);
      }
      if(!isset($cols_length[$index]) or $cols_length[$index] < $length) {
        $cols_length[$index] = $length;
      }
      $index++;
    }
  }
  return $cols_length;
}

/**
 * Count total data width;
 */
function _render_table_cut_table_index($cols_length, $col_width_total, $max_width){
  $cut_table_index = -1;  
  if($col_width_total > $max_width) {
    $max_length = 0;
    foreach($cols_length as $index => $length) {
      if($length > $max_length){
        $cut_table_index = $index;
        $max_length = $length;
      }
    }
  }
  return $cut_table_index;
}

/**
 * Generate mask for row;
 */
function _render_table_mask($cols_length, $cut_table_index, $width_diff, $delimiter, $delimiter_left, $delimiter_right){
  $mask = $delimiter_left;
  foreach($cols_length as $index => $length) {
    if($cut_table_index == $index){
      if ($length - $width_diff > 0 ){
        $mask .= '%-' . ($length - $width_diff). 's' . $delimiter;
      }
      else{
        $mask .= '%-' . $length. 's' . $delimiter; 
      }
    }
    else{
      $mask .= '%-' . $length. 's' . $delimiter;
    }
    
  }
  $mask = mb_substr($mask, 0, - mb_strlen($delimiter));
  $mask .= $delimiter_right ."\n";
  return $mask;
}

/**
 * Assign $arguments and $options based on 
 * $command['arguments'] & $command[options]
 *
 */
function render_table($variables) {
  $header = FALSE;
  if(isset($variables['header'])){
    $header = $variables['header'];
  }
  $rows = $variables['rows'];
  $delimiter = isset($variables['delimiter']) ? $variables['delimiter'] : " | ";
  $delimiter_left = isset($variables['delimiter_left']) ? $variables['delimiter_left'] : "";
  $delimiter_right = isset($variables['delimiter_right']) ? $variables['delimiter_right'] : "";
  $max_width = exec('tput cols');
  

  if($header){
    $header_args = array();
    foreach($header as $col) {
      if(isset($col['data'])){
        $header_args[] = table_color_text($col);
      }
      else{
        $header_args[] = $col;
      }
    }
  
    echo vsprintf($mask, $header_args);
    echo str_repeat('-', $col_width_total - $width_diff) . "\n";
  }

  if(empty($rows)){
    $mask = $delimiter . '%-' . ($max_width - mb_strlen($delimiter) *2 ) . 's' . $delimiter ."\n";
    echo vsprintf($mask, array('no data'));
    return;
  }
  
  
 
  
  $cols_length = _render_table_row_col_length($rows);

  $col_width_total = 0;
  foreach($cols_length as $length) {
    $col_width_total += $length + mb_strlen($delimiter);
  }
  $col_width_total += mb_strlen($delimiter);
  
  
  $cut_table_index = _render_table_cut_table_index($cols_length, $col_width_total, $max_width);
  $width_diff = 0;
  if(-1 != $cut_table_index) {
    $width_diff = $col_width_total - $max_width;
  }
  
  $mask = _render_table_mask($cols_length, $cut_table_index, $width_diff, $delimiter, $delimiter_left, $delimiter_right);
  
  foreach($rows as $row) {
    $print_args = array();
    $repeat = FALSE;
    foreach($row as $index => $col) {
      $data = '';
      if(isset($col['data'])){
        $data = table_color_text($col);
      }
      else{
        $data = $col;
      }
      if($cut_table_index == $index) {
        if(mb_strlen($data) - $width_diff > 0) {
          $repeat = array(
            'index' => $index,
            'data' => mb_substr($data, -$width_diff),
            'width_diff' => $width_diff,
            'length' => mb_strlen($data) - $width_diff,
          );
          $data = substr($data, 0 , -$width_diff);
        }
        else{
          $data = $col;
        }
      }
      $print_args[] = $data;
    }
    echo vsprintf($mask, $print_args);
    if($repeat) {
      $count = count($row);
      _row_repeat($mask, $repeat, $count);
    }
  }
}

function _row_repeat($mask, $repeat, $count) {
  $print_args = array();
  $repeat_more = FALSE;
//  print_r($repeat);
  for($index =0; $index < $count; $index++){
    if($repeat['index'] == $index){
      if(mb_strlen($repeat['data']) > $repeat['length']){
        $print_args[] = mb_substr($repeat['data'], 0, $repeat['length']);
        $repeat_more = $repeat;
        $repeat_more['data'] = mb_substr($repeat['data'], $repeat['length']);
      }
      else{
        $print_args[] = $repeat['data'];
      }
    }
    else{
      $print_args[] = '';
    }
  }
  echo vsprintf($mask, $print_args);
  if($repeat_more){
    _row_repeat($mask, $repeat_more, $count);
  }
}
