#countries {
  polygon-fill: #eee;
  line-color: #ccc;
  line-width: 1;

  .new {
    polygon-fill: #CCC;
  }

  .new[zoom > 5] {
    line-width:0.5;

    [NAME='United States'] {
      polygon-fill:#AFC;
    }
  }
}
