<!DOCTYPE html>
<html>
<head>
    <title>Date and Time</title>
    <style>
        body {
            background-color: #222;
            color: #fff;
            font-family: Arial, sans-serif;
            text-align: center;
            padding-top: 100px;
        }

        h1 {
            font-size: 24px;
            margin-bottom: 10px;
        }

        h2 {
            font-size: 18px;
            margin-bottom: 20px;
        }
    </style>
</head>
<body>
    <h1>Welcome to webserv "Date and Time"</h1>
    <h2>Current Date and Time:</h2>
    <?php
        date_default_timezone_set('UTC'); // Set the timezone according to your preference
        $currentDateTime = date('Y-m-d H:i:s');
        echo "<p>{$currentDateTime}</p>";
    ?>
    <p>This is a minimalistic index.php file in a dark theme. It displays the current date and time dynamically using PHP.</p>
    <p>Feel free to customize and enhance it further to meet your needs.</p>
</body>
</html>