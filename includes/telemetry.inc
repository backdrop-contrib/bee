<?php
/**
 * @file
 *
 * Custom implemention of Backdrop telemetry functions as Bee cannot use module
 * hooks.
 */

define('BEE_TELEMETRY_DESTINATION_URL', "https://telemetry.backdropcms.org/post");

/**
 * Main calling function.
 *
 * Ensures telemetry is only sent if it is enabled for the site and it hasn't
 * been sent in the last day.
 */
function bee_invoke_telemetry() {
  if (bee_telemetry_enabled()) {
    // Check when Bee telemetry was last sent.
    $last_run = bee_telemetry_get_last();
    $interval = 86400; // One day in seconds.
    if (REQUEST_TIME - $last_run > $interval) {
      $send_status = bee_telemetry_send();
      if ($send_status) {
        $set_last = bee_telemetry_set_last();
        if (!$set_last) {
          bee_instant_message("Unable to set state 'bee_telemetry_last'.", 'log');
        }
      }
      else {
        bee_instant_message("Failure sending bee telemetry.", 'log');
      }
    }
  }
}

/**
 * Send the telemetry to the server.
 */
function bee_telemetry_send() {
  global $_bee_backdrop_root;
  $send_status = FALSE;

  // Get the telemetry data.
  $telemetry_data = json_encode(bee_telemetry_get_data());

  if ($site_key = bee_telemetry_get_site_key()) {
    // If the site key was successfully retrieved (i.e. $site_key != FALSE)
    // then attempt to send the data.
    if ($curl_handle = curl_init(BEE_TELEMETRY_DESTINATION_URL)) {
      // Prepare headers for curl request.
      $curlopt_httpheader = array();
      $curlopt_httpheader[] = 'Content-Type: application/json';
      $curlopt_httpheader[] = 'X-Site-Key: ' . $site_key;

      // Prepare the options for curl request.
      curl_setopt($curl_handle, CURLOPT_HTTPHEADER, $curlopt_httpheader);
      curl_setopt($curl_handle, CURLOPT_HEADER, TRUE);
      curl_setopt($curl_handle, CURLOPT_RETURNTRANSFER, TRUE);
      curl_setopt($curl_handle, CURLOPT_POST, TRUE);
      curl_setopt($curl_handle, CURLOPT_POSTFIELDS, $telemetry_data);
      curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);

      // Execute the curl request and store the output in variable.
      $response = curl_exec($curl_handle);

      // Get the response code.
      $response_code = curl_getinfo($curl_handle, CURLINFO_HTTP_CODE);

      // Extract the header and body (if it exists).
      $response_header_length = curl_getinfo($curl_handle, CURLINFO_HEADER_SIZE);
      $response_header = substr($response, 0, $response_header_length);
      $response_body = substr($response, $response_header_length);
      $response_headers = bee_get_headers_from_curl_response($response_header);

      // Close the curl request.
      curl_close($curl_handle);

      // Debug message of the header array.
      bee_instant_message('Telemetry $response_headers:', 'debug', $response_headers);

      // If there is a body then provide a debug message of the body array.
      if ($response_body) {
        bee_instant_message('Telemetry $response_body:', 'debug', (array) json_decode($response_body));
      }

      // Check the response code and if it has been successful and there is no
      // body, set the $send_status to TRUE.
      bee_instant_message('Telemetry: $response_code:' . $response_code, 'debug');
      if ($response_code == 200 && empty($response_body)) {
        $send_status = TRUE;
      }
    }
  }
  return $send_status;
}

/**
 * Helper function to turn the headers from a curl response into an associative
 * array.
 *
 * To get response header content you need to get the header size from the curl
 * handle:
 *
 * @code
 *   $response = curl_exec($curl_handle);
 *   $response_header_length = curl_getinfo($curl_handle, CURLINFO_HEADER_SIZE);
 *   $response_header_content = substr($response, 0, $response_header_length);
 * @endcode
 *
 * @param string $response_header_content
 *   The header content extracted from the response.
 *
 * @return array
 *   An associative array of header content.
 */
function bee_get_headers_from_curl_response($response_header_content) {
  $headers = array();

  // Split the string on every "double" new line.
  $arrRequests = explode("\r\n\r\n", $response_header_content);

  // Loop of response headers. The "count() -1" is to avoid an empty row for
  // the extra line break before the body of the response.
  for ($index = 0; $index < count($arrRequests) - 1; $index++) {
    // Loop through each line.
    foreach (explode("\r\n", $arrRequests[$index]) as $line_index => $line) {
      if ($line_index === 0) {
        $headers[$index]['http_code'] = $line;
      }
      else {
        list ($key, $value) = explode(': ', $line);
        $headers[$index][$key] = $value;
      }
    }
  }
  // If there is only one top level then remove the top level.
  if (count($headers) == 1) {
    $headers = $headers[0];
  }
  return $headers;
}

/**
 * Check whether the telemetry module is enabled for the site.
 *
 * @return bool
 *   TRUE if module is enabled, FALSE otherwise.
 */
function bee_telemetry_enabled() {
  global $_bee_bootstrap_level, $settings;
  $status = FALSE;

  $bee_telemetry_override_setting = isset($settings['telemetry_enabled']) ? $settings['telemetry_enabled'] : TRUE;
  if (!$bee_telemetry_override_setting) {
    return $status;
  }

  if ($_bee_bootstrap_level == BEE_BOOTSTRAP_FULL) {
    // If full bootstrap we can use the Backdrop function.
    $status = module_exists('telemetry');
  }
  else {
    // If a lower level of bootstrap we can attempt to check the database.
    if (class_exists('Database')) {
      $db_connection = Database::getConnectionInfo();
      if (!empty($db_connection['default'])) {
        try {
          $query = "SELECT `status` FROM {system} WHERE `name` = 'telemetry'";
          $status = (bool) db_query($query)->fetchField();
        }
        // If the database query fails then set status to false.
        catch (Exception $e) {
          $status = FALSE;
        }
      }
    }
  }
  return $status;
}

/**
 * Get the timestamp of the last time Bee telemetry was sent.
 *
 * @return int|bool
 *   An integer timestamp if it exists. FALSE otherwise.
 */
function bee_telemetry_get_last() {
  $telemetry_last = FALSE;
  $state_value = bee_telemetry_get_state('bee_telemetry_last');
  if (!is_null($state_value)) {
    $telemetry_last = $state_value;
  }
  return $telemetry_last;
}

/**
 * Set the timestamp with the current request time.
 */
function bee_telemetry_set_last() {
  global $_bee_bootstrap_level;
  $telemetry_last = FALSE;
  $state_name = 'bee_telemetry_last';
  if ($_bee_bootstrap_level == BEE_BOOTSTRAP_FULL) {
    // If full bootstrap we can use the Backdrop function.
    state_set($state_name, REQUEST_TIME);
    $telemetry_last = TRUE;
  }
  else {
    // If a lower level of bootstrap we can attempt to check the database.
    if (class_exists('Database')) {
      $db_connection = Database::getConnectionInfo();
      if (!empty($db_connection['default'])) {
        try {
          db_merge('state')
          ->key(array('name' => $state_name))
          ->fields(array('value' => serialize(REQUEST_TIME)))
          ->execute();
          $telemetry_last = TRUE;
        }
        // If the database query fails then set status to false.
        catch (Exception $e) {
          $telemetry_last = FALSE;
        }
      }
    }
  }
  return $telemetry_last;
}

/**
 * Generate the site key.
 *
 * @return string|bool
 *   Return the site key if possible to retrieve the private key; FALSE
 *   otherwise.
 */
function bee_telemetry_get_site_key() {
  $site_key = FALSE;
  $private_key = bee_telemetry_get_state('private_key');
  if (!is_null($private_key)) {
    $site_key = backdrop_hmac_base64('telemetry_site_key', $private_key);
  }
  return $site_key;
}

/**
 * Compile the data into an array that meets the telemetry requirements.
 *
 * @return array
 *   An array of data with the Bee version, the PHP (CLI) version and whether
 *   or not the site is a multisite.
 */
function bee_telemetry_get_data() {
  global $_bee_multisites;
  $data = array(
    'bee' => array(
      'version' => BEE_VERSION,
      'bee_version' => BEE_VERSION,
      'php_version' => PHP_VERSION,
      'multisite' => !empty($_bee_multisites) ? 'Yes' : 'No',
    ),
  );
  return $data;
}

/**
 * Helper function to get state values whether Backdrop is fully
 * bootstrapped or not.
 *
 * @param string $state_name
 *   The name of the state to get or set.
 * @return mixed
 *   Return the state unserialized value or NULL.
 */
function bee_telemetry_get_state($state_name) {
  global $_bee_bootstrap_level;
  $state_value = NULL;
  if ($_bee_bootstrap_level == BEE_BOOTSTRAP_FULL) {
    // If full bootstrap we can use the Backdrop function.
    $state_value = state_get($state_name);
  }
  else {
    // If a lower level of bootstrap we can attempt to check the database.
    if (class_exists('Database')) {
      $db_connection = Database::getConnectionInfo();
      if (!empty($db_connection['default'])) {
        try {
          $serialized = db_query('SELECT value FROM {state} WHERE name = :state_name', array(
            ':state_name' => $state_name,
          ))->fetchField();
          $state_value = unserialize($serialized);
        }
        // If the database query fails then set status to false.
        catch (Exception $e) {
          $state_value = NULL;
        }
      }
    }
  }
  return $state_value;
}
