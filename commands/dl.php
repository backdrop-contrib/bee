<?php
/**
 * dl_project figures out the latest github release from either
 * backdrop/backdrop for core or backdrop-contrib for contrib modules
 *
 * @param $project
 *   array of project(s) you wish to download, i.e. redirect or webform
 *
 * @param $path
 *   string backdrop root, i.e. 'var/www/backdrop'
 */
function dl_project($project, $path) {
  //$backdrop_path = '/Users/geoff/Sites/backdrop';
  if ($project != 'backdrop') {
    $html = get_content_from_github(
      "https://github.com/backdrop-contrib/$project/releases/latest"
    );

    $html = explode("\"", $html);
    $url = $html[1];
    $latest = explode('/', $url);
    $latest = array_reverse($latest);
    if (file_exists($path . '/modules/contrib')) {
      $module_path = $path . '/modules/contrib';
    }
    else {
      $module_path = $path . '/modules';
    }

    exec(
      "wget --directory-prefix $module_path https://github.com/backdrop-contrib/$project/releases/download/$latest[0]/$project.zip"
    );
    // extract the zip file
    exec(
      "unzip $module_path/$project.zip -d $module_path"
    );
    // remove the zip file
    exec(
      "rm $module_path/$project.zip"
  );
  }
  // Downloading backdrop itself is a special case.
  elseif ($project == 'backdrop') {
    $html = get_content_from_github(
      "https://github.com/backdrop/backdrop/releases/latest"
    );

    $html = explode("\"", $html);
    print_r($html);
    $url = $html[1];
    $latest = explode('/', $url);
    $latest = array_reverse($latest);

    // get the module
    exec(
      "wget https://github.com/$project/$project/releases/download/$latest[0]/backdrop.zip"
    );
    // extract the zip file
    exec(
      "unzip backdrop.zip"
    );
    // remove the zip file
    exec(
      "rm backdrop.zip"
    );
  }

  print "Successfully downloaded.\n";
}

/* gets url */
function get_content_from_github($url) {
  $ch = curl_init();
  curl_setopt($ch,CURLOPT_URL,$url);
  curl_setopt($ch,CURLOPT_RETURNTRANSFER,1);
  curl_setopt($ch,CURLOPT_CONNECTTIMEOUT,1);
  $content = curl_exec($ch);
  curl_close($ch);
  return $content;
}
?>