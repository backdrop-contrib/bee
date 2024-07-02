<?php
/**
 * @file
 *
 * Copies of Backdrop functions for processing info files.
 *
 * As download may need to operate to download Backdrop or before Backdrop is
 * installed, we cannot rely on these Backdrop functions being available.
 */

 /**
  * @see backdrop_parse_info_format()
  */
function bee_parse_info_format($data, $process_sections = FALSE) {
  $info = array();

  // Separate by sections and parse individually if requested.
  if ($process_sections) {
    // Remove leading a trailing comments before groups.
    $data = trim(preg_replace('/;.*$/m', '', $data));

    // Split the content by the groups present.
    $split = preg_split('/^\[(.*)\]\s*$/m', $data, -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
    $split_count = count($split);

    // If sections are found, parse each individually and add to the result.
    if ($split_count > 1) {
      $info_sections = array();
      for ($n = 0; $n < $split_count; $n = $n + 2) {
        $info_sections[$split[$n]] = bee_parse_info_format($split[$n + 1], FALSE);
      }
      return $info_sections;
    }
  }

  if (preg_match_all('
    @^\s*                           # Start at the beginning of a line, ignoring leading whitespace
    ((?:
      [^=;\[\]]|                    # Key names cannot contain equal signs, semi-colons or square brackets,
      \[[^\[\]]*\]                  # unless they are balanced and not nested
    )+?)
    \s*=\s*                         # Key/value pairs are separated by equal signs (ignoring white-space)
    (?:
      ("(?:[^"]|(?<=\\\\)")*")|     # Double-quoted string, which may contain slash-escaped quotes/slashes
      (\'(?:[^\']|(?<=\\\\)\')*\')| # Single-quoted string, which may contain slash-escaped quotes/slashes
      ([^\r\n]*?)                   # Non-quoted string
    )\s*$                           # Stop at the next end of a line, ignoring trailing whitespace
    @msx', $data, $matches, PREG_SET_ORDER)) {
    foreach ($matches as $match) {
      // Fetch the key and value string.
      $i = 0;
      $parts = array();
      foreach (array('key', 'value1', 'value2', 'value3') as $var) {
        $parts[$var] = isset($match[++$i]) ? $match[$i] : '';
      }
      $value = stripslashes(substr($parts['value1'], 1, -1)) . stripslashes(substr($parts['value2'], 1, -1)) . $parts['value3'];

      // Parse array syntax.
      $keys = preg_split('/\]?\[/', rtrim($parts['key'], ']'));
      $last = array_pop($keys);
      $parent = &$info;

      // Create nested arrays.
      foreach ($keys as $key) {
        if ($key == '') {
          $key = count($parent);
        }
        if (!isset($parent[$key]) || !is_array($parent[$key])) {
          $parent[$key] = array();
        }
        $parent = &$parent[$key];
      }

      // Handle PHP constants.
      if (preg_match('/^\w+$/', $value) && defined($value)) {
        $value = constant($value);
      }

      // Insert actual value.
      if ($last == '') {
        $last = count($parent);
      }
      $parent[$last] = $value;
    }
  }

  return $info;
}

/**
 * @see backdrop_parse_dependency
 */
function bee_parse_dependency($dependency) {
  $value = array();
  // Split out the optional project name.
  if (strpos($dependency, ':')) {
    list($project_name, $dependency) = explode(':', $dependency);
    $value['project'] = $project_name;
  }
  // We use named subpatterns and support every op that version_compare
  // supports. Also, op is optional and defaults to equals.
  $p_op = '(?P<operation>!=|==|=|<|<=|>|>=|<>)?';
  // Core version is always optional: 1.x-2.x and 2.x is treated the same.
  $p_core = '(?:' . preg_quote(BACKDROP_CORE_COMPATIBILITY) . '-)?';
  $p_major = '(?P<major>\d+)';
  // By setting the minor version to x, branches can be matched.
  $p_minor = '(?P<minor>(?:\d+|x)(?:-[A-Za-z]+\d+)?)';
  $p_patch = '(?P<patch>(?:\d+|x)(?:-[A-Za-z]+\d+)?)?';
  $parts = explode('(', $dependency, 2);
  $value['name'] = trim($parts[0]);
  if (isset($parts[1])) {
    $value['original_version'] = '(' . $parts[1];
    foreach (explode(',', $parts[1]) as $version) {
      if (preg_match("/^\s*$p_op\s*$p_core$p_major\.$p_minor\.?$p_patch/", $version, $matches)) {
        $op = !empty($matches['operation']) ? $matches['operation'] : '=';
        if ($matches['minor'] == 'x') {
          // Backdrop considers "2.x" to mean any version that begins with
          // "2" (e.g. 2.0, 2.9 are all "2.x"). PHP's version_compare(),
          // on the other hand, treats "x" as a string; so to
          // version_compare(), "2.x" is considered less than 2.0. This
          // means that >=2.x and <2.x are handled by version_compare()
          // as we need, but > and <= are not.
          if ($op == '>' || $op == '<=') {
            $matches['major']++;
          }
          // Equivalence can be checked by adding two restrictions.
          if ($op == '=' || $op == '==') {
            $value['versions'][] = array(
              'op' => '<',
              'version' => ($matches['major'] + 1) . '.x',
            );
            $op = '>=';
          }
        }

        if (isset($matches['patch']) && ($matches['patch'] === '0' || $matches['patch'])) {
          if ($matches['patch'] == 'x' && $matches['minor'] !== 'x') {
            // See comments above about "x" in minor.
            // Same principle applies to patch in relation to minor.
            if ($op == '>' || $op == '<=') {
              $matches['minor']++;
            }
            if ($op == '=' || $op == '==') {
              $value['versions'][] = array(
                'op' => '<',
                'version' => $matches['major'] . '.' . ($matches['minor'] + 1) . '.x',
              );
              $op = '>=';
            }
          }
        }
        $version = $matches['major'] . '.' . $matches['minor'];
        $version .= (isset($matches['patch']) && ($matches['patch'] === '0' || $matches['patch'])) ? '.' . $matches['patch'] : '';
        $value['versions'][] = array('op' => $op, 'version' => $version);
      }
    }
  }
  return $value;
}
