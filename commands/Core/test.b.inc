<?php
/**
 * Implementation of hook_b_command().
 */
function test_b_command() {
  $items['st'] = array(
    'description' => 'This is test example how to add your own callback on command.',
    'callback' => 'test_b_callback',
  );
  return $items;
}

function test_b_callback(){
  if(defined('BACKDROP_ROOT')){
    return array(
      '#type' => 'text',
      '#color' => 'bold_cyan',
      'value' => 'Backdrop CMS Installation detected; wooot;',
    );
  }
  return array(
      '#type' => 'text',
      '#color' => 'brown',
      'value' => 'No Backdrop installation found. :(',
    );
}
