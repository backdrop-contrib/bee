<?php
/**
 * Implementation of hook_b_command().
 */
function modules_b_command() {
  $items['pm-list'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Download contrib package.',
    'arguments' => array(
      'package' => 'Package to download.',
    ),
    'options' => array(
      'root' => 'Backdrop ROOT directory',
    ),
    'callback' => 'modules_b_callback',
    'aliases' => array('pml'),
  );
  return $items;
}

function modules_b_callback($arguments, $options){
  $files = system_rebuild_module_data();
  $rows = array();

  foreach($files as $file) {
    if(isset($file->info['hidden']) && $file->info['hidden']){
      continue;
    }
    $status = array(
      'data' => 'Disabled',
      '#color' => 'red',
    );
    if($file->status){
      $status = array(
        'data' => 'Enabled',
        '#color' => 'green',
      );
    }
    $rows[] = array(
      $file->info['package'],
      $file->info['name'],
      $file->type,
      $status,
      $file->info['version'],
    );
  }

  $header = array(
    'Package',
    'Name',
    'Type',
    'Status',
    'Version',
  );

  return array(
    '#type' => 'table',
    'rows' => $rows,
    'header' => $header,
    'delimiter' => '  ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );
}
