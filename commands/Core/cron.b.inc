<?php
/**
 * @file
 * Command(s) for running cron.
 */

/**
 * Implements hook_b_command().
 */
function cron_b_command() {
  $items['cron'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Process cron.',
    'callback' => 'cron_command_process',
  );

  return $items;
}

/**
 * Callback function for hook_b_command().
 */
function cron_command_process($arguments, $options) {
  $success = backdrop_cron_run();
  if ($success) {
    b_message('Cron processed.', 'success');
  }
  else {
    b_message('Cron is already running.', 'error');
  }
}
