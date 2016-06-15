<?php
/**
 * Implementation of hook_b_command().
 */
 
 function cron_b_command() {
  $items = array();

  $items['cron'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Process cron.',
    'callback' => 'cron_command_process',
  );
  return $items;
}


function cron_command_process($arguments, $options) {
  backdrop_cron_run();
  return array(
    '#type' => 'text',
    '#color' => 'green',
    'value' => 'Cron processed.',
  );
}
