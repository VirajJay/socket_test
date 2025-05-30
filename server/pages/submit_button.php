<?php
$submitted = false;

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    
    $submitted = true;
    $lolwhat = 43;
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>Submit Button Example</title>
</head>
<body>

<h2>PHP Submit Button Example</h2>

<form method="post" action="">
  <label for="fname">First name:</label><br>
  <input type="text" id="fname" name="first_name"><br>
  <label for="lname">Last name:</label><br>
  <input type="text" id="lname" name="last_name">
  <button type="submit" name="submit_button" value="clicked">Click Me</button>
</form>

<?php if ($submitted): ?>
    <p><strong>Form was submitted successfully!</strong></p>
<?php endif; ?>

</body>
</html>
