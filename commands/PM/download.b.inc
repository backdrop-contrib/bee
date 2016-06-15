<?php
/**
 * Implementation of hook_b_command().
 */
function download_b_command() {
  $items['dl'] = array(
    'description' => 'Download contrib package.',
    'arguments' => array(
      'package' => 'Package to download.',
    ),
    'options' => array(
      'root' => 'Backdrop ROOT directory',
    ),
    'callback' => 'download_b_callback',
  );
  return $items;
}

function download_b_callback($arguments, $options) {
  if(!isset($arguments['package'])){
    return array(
      '#type' => 'text',
      '#color' => 'red',
      'value' => 'Please provide package name',
    );
  }
  if(!defined('BACKDROP_ROOT') && !isset($options['root'])){
    return array(
      '#type' => 'text',
      '#color' => 'red',
      'value' => 'Please provide --root option or change directory to BackdropCMS folder',
    );
  }
  
  if($arguments['package'] == 'backdrop'){
    $link = get_project_link('backdrop', $arguments['package']);
    if(isset($options['root'])) {
      $download_target = $options['root'];
    }else{
      $download_target = defined('BACKDROP_ROOT');
    }
    exec('wget -q --directory-prefix ' . $download_target . ' ' . $link);
    exec('unzip ' . $download_target . '/' . $arguments['package'] . '.zip -d ' .$download_target);
    unlink($download_target . '/' . $arguments['package'] . '.zip');
    return array(
      '#type' => 'text',
      '#color' => 'green',
      'value' => 'BackdropCMS downloaded!',
    );
  }
  else{
    if($link = get_project_link('backdrop-contrib', $arguments['package'])) {
      if(isset($options['root'])) {
        $download_target = $options['root'] . '/modules';
      }else{
        $download_target = defined('BACKDROP_ROOT') . '/modules';
      }
      if(file_exists($download_target . '/contrib')) {
        $download_target = $download_target . '/contrib';
      }
      echo $download_target;
      exec('wget -q --directory-prefix ' . $download_target . ' ' . $link);
      exec('unzip ' . $download_target . '/' . $arguments['package'] . '.zip -d ' .$download_target);
      unlink($download_target . '/' . $arguments['package'] . '.zip');
      return array(
      '#type' => 'text',
      '#color' => 'green',
      'value' => $arguments['package'] . ' downloaded!',
    );
    }
    else{
       return array(
        '#type' => 'text',
        '#color' => 'red',
        'value' => 'There is no such project or no releases!',
      );
      
    }
  }
}

function get_project_link($owner,$repo) {
  $context = stream_context_create(
    array(
      "http" => array(
        'method'=>"GET",
        'header'=>"Accept-language: en\r\n" .
              "User-Agent: B.project\r\n"
      )
    )
  );
  $content = file_get_contents('https://api.github.com/repos/' . $owner . '/' . $repo .'/releases/latest', false, $context);
  $json = json_decode($content, TRUE);
  
  if(isset($json['assets'][0]['browser_download_url'])){
    return $json['assets'][0]['browser_download_url'];
  }
  return FALSE;
}
