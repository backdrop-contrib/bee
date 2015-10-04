<?php
/**
 * dl_project figures out the latest github release from either
 * backdrop/backdrop for core or backdrop-contrib for contrib modules
 * @param $project
 *   array of project(s) you wish to download, i.e. redirect or webform
 */
function dl_project($project) {
  $backdrop_path = '/Users/geoff/Sites/backdrop';
  if ($project != 'backdrop') {
    $html = get_content_from_github(
      "https://github.com/backdrop-contrib/$project/releases/latest"
    );

    $html = explode("\"", $html);
    $url = $html[1];
    $latest = explode('/', $url);
    $latest = array_reverse($latest);

    exec(
      "wget --directory-prefix $backdrop_path/modules/ https://github.com/backdrop-contrib/$project/releases/download/$latest[0]/$project.zip"
    );
  }
  elseif ($project == 'backdrop') {
    $html = get_content_from_github(
      "https://github.com/backdrop/backdrop/releases/latest"
    );

    $html = explode("\"", $html);
    print_r($html);
    $url = $html[1];
    $latest = explode('/', $url);
    $latest = array_reverse($latest);

    exec(
      "wget https://github.com/$project/$project/releases/download/$latest[0]/backdrop.zip"
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