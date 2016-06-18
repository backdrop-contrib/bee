<?php
/**
 * Implementation of hook_b_command().
 */
function config_b_command() {
  $items['config-list'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Show a list of configs.',
    'callback' => 'config_list_callback',
  );
  $items['config-get'] = array(
    'bootstrap' => B_BOOTSTRAP_FULL,
    'description' => 'Chow config settings.',
    'callback' => 'config_get_callback',
    'arguments' => array(
      'config_name' => array(
        'description' => 'Config name.',
        'multiple' => TRUE,
        'required' => TRUE,
      ),
    ),
  );

  return $items;
}

/**
 * config-get callback. Display list of configs.
 */
function config_get_callback($arguments, $options){

  $config_dir = config_get_config_directory();
  $config_storage = new ConfigFileStorage($config_dir);

  foreach($arguments['config_name'] as $config_name){
    $config = $config_storage->read($config_name);

    $info =  config_get_info($config_name);

    $rows = array();
    $label ='';
    if(isset($info['label'])){
      $label = $info['label'];
    }
    if(isset($info['label_key'])){
      $label = config_get($config_name,$info['label_key']);
    }

    if(isset($info['label_callback'])){
      $label = $info['label_callback']($config, $config_name);
    }

    $group ='';
    if(isset($info['group'])){
      $group = $info['group'];
    }

    $rows[] = array(
      'File name',
      $config_name,
    );
    $rows[] = array(
      'Label',
      $label,
    );

    $rows[] = array(
      'Group',
      $group,
    );

    $output[] = array(
      '#type' => 'text',
      'value' => "\n",
    );
    $output[] = array(
      '#type' => 'table',
      'rows' => $rows,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => ' ',
    );
    $output[] = array(
      '#type' => 'text',
      'value' => "\n",
    );
    $output[] = prepare_config_table($config);
  }
  return $output;
}

/**
 * Prepare config values to be displayed.
 */
function prepare_config_table($config) {
  $rows = array();
  foreach($config as $key => $value) {
    if(is_string($value)) {
      $rows[] = array(
        $key,
        $value,
      );
      continue;
    }

    if(is_bool($value)){
      $rows[] = array(
        $key,
        $value ? 'TRUE' : 'FALSE',
      );
      continue;
    }

    if(is_array($value)){
      $list = array();
      foreach($value as $ar_name => $ar_value){;
        if(is_array($ar_value)){
          $ar_value = 'Array()';
        }
        $list[] = $ar_name . " = " . $ar_value;
      }
      $rows[] = array(
        $key,
        implode(" ", $list),
      );
    }
  }

  $header = array(
    'Name',
    'Value',
  );

  return array(
    '#type' => 'table',
    'rows' => $rows,
    'header' => $header,
    'delimiter' => ' : ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );
}
/**
 * config-list callback. Display list of configs.
 */
function config_list_callback($arguments, $options){
  $config_dir = config_get_config_directory();
  $config_storage = new ConfigFileStorage($config_dir);
  $config_files = array();

  $rows = array();

  foreach ($config_storage->listAll() as $config_name) {
    $info =  config_get_info($config_name);

    $label ='';
    if(isset($info['label'])){
      $label = $info['label'];
    }
    if(isset($info['label_key'])){
      $label = config_get($config_name,$info['label_key']);
    }

    if(isset($info['label_callback'])){
      $config = $config_storage->read($config_name);
      $label = $info['label_callback']($config, $config_name);
    }

    $group ='';
    if(isset($info['group'])){
      $group = $info['group'];
    }

    $rows[] = array(
      $config_name,
      $label,
      $group,
    );

  }
  $header = array(
    bt('File name'),
    bt('Label'),
    bt('Group'),
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
