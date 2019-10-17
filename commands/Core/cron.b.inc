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
  $success = backdrop_cron_run();
  if ($success) {
    b_set_message('Cron processed.', 'success');
  }
  else {
    b_set_message('Cron is already running.', 'error');
  }
}
