<?php
/**
 * Implementation of hook_b_command().
 */
function test_b_command() {
  $items['core-status'] = array(
    'description' => 'This is test example how to add your own callback on command.',
    'callback' => 'test_b_callback',
    'aliases' => array('status', 'st'),
  );
  return $items;
}

function test_b_callback($arguments, $options) {
  if(defined('BACKDROP_ROOT')){
    b_set_message('Backdrop CMS Installation detected; wooot;', 'status');
  }
  else{
    b_set_message('No Backdrop installation found. :(', 'warning');
  }
}
