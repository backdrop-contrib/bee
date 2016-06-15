<?php
/**
 * Implementation of hook_b_command().
 */
 
 function cache_b_command() {
  $items = array();

  $items['cache-clear'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Clear a specific cache, or all drupal caches.',
    'arguments' => array(
      'type' => 'The particular cache to clear. Omit this argument to choose from available caches.',
    ),
    'callback' => 'cache_command_clear',
    'aliases' => array('cc'),
  );
  return $items;
}


function cache_command_clear($arguments, $options) {
  if(!isset($arguments['type'])) {
    backdrop_flush_all_caches();
    return array(
      '#type' => 'text',
      '#color' => 'green',
      'value' => 'All caches cleared.',
    );
  }
}
